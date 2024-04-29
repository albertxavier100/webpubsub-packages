#pragma once

#include "nlohmann/json.hpp"

#include "WebPubSub/Protocols/Requests/SendEventRequest.hpp"
#include "eventpp/callbacklist.h"
#include "webpubsub/client/concepts/websocket_factory_c.hpp"
#include "webpubsub/client/credentials/client_credential.hpp"
#include "webpubsub/client/detail/async/exclusion_lock.hpp"
#include "webpubsub/client/detail/client/failed_connection_context.hpp"
#include "webpubsub/client/detail/client/group_context.hpp"
#include "webpubsub/client/detail/client/retry_context.hpp"
#include "webpubsub/client/detail/logging/log.hpp"
#include "webpubsub/client/detail/services/client_channel_service.hpp"
#include "webpubsub/client/detail/services/client_lifetime_service.hpp"
#include "webpubsub/client/detail/services/client_receive_service.hpp"
#include "webpubsub/client/detail/services/client_send_service.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_events.hpp"
#include "webpubsub/client/detail/services/models/transition_context.hpp"
#include "webpubsub/client/exceptions/exception.hpp"
#include "webpubsub/client/models/client_options.hpp"

namespace webpubsub {
template <webpubsub_protocol_t protocol_t, typename websocket_factory_t,
          typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client {
  template <typename t = void> using async_t = io::awaitable<t>;

public:
  client(io::strand<io::io_context::executor_type> &strand,
         const client_credential &credential,
         const client_options<protocol_t> &options,
         websocket_factory_t &websocket_factory, const std::string &logger_name)
      : log_(logger_name), options_(options),
        lifetime_(strand, credential, websocket_factory, options, log_),
        receive_(strand, options, log_), send_(strand, options, log_),
        transition_context_(strand, lifetime_, receive_, send_, log_),
        on_connected(transition_context_.on_connected),
        on_disconnected(transition_context_.on_disconnected),
        on_group_data(transition_context_.on_group_data),
        on_server_data(transition_context_.on_server_data),
        on_rejoin_group_failed(transition_context_.on_rejoin_group_failed),
        on_stopped(transition_context_.on_stopped) {
    setup_reconnect_callback(strand);
    io::co_spawn(
        strand,
        receive_.async_start_group_data_response_handler(&transition_context_),
        io::detached);
    io::co_spawn(
        strand,
        receive_.async_start_server_data_response_handler(&transition_context_),
        io::detached);
  }

  eventpp::CallbackList<void(const connected_context)> &on_connected;
  eventpp::CallbackList<void(const disconnected_context)> &on_disconnected;
  eventpp::CallbackList<void(const group_data_context)> &on_group_data;
  eventpp::CallbackList<void(const server_data_context)> &on_server_data;
  eventpp::CallbackList<void(const rejoin_group_failed_context)>
      &on_rejoin_group_failed;
  eventpp::CallbackList<void(const stopped_context)> &on_stopped;

  auto async_start() -> async_t<> {
    co_await transition_context_.async_raise_event(
        detail::to_connecting_state{});
    auto event = detail::to_connected_state{};
    co_await transition_context_.async_raise_event(std::move(event));
  }

  auto async_stop() -> async_t<> {
    spdlog::trace("async_stop -- beg");
    co_await transition_context_.async_raise_event(detail::to_stopping_state{});
    co_await transition_context_.async_raise_event(detail::to_stopped_state{});
    spdlog::trace("async_stop -- end");
  }

  auto async_cancel() -> async_t<> {
    co_await send_.async_cancel_sequence_id_loop_coro();
    co_await receive_.async_cancel_message_loop_coro();
    co_await lifetime_.async_close();
  }

  template <typename data_t>
  auto async_send_to_group(const SendToGroupRequest<data_t> request,
                           bool fire_and_forget = false)
      -> async_t<const request_result> {
    return send_.async_retry_send(request, &transition_context_,
                                  fire_and_forget);
  }

  template <typename data_t>
  auto async_send_event(const SendEventRequest<data_t> request,
                        bool fire_and_forget = false)
      -> async_t<const request_result> {
    return send_.async_retry_send(request, &transition_context_,
                                  fire_and_forget);
  }

  auto async_join_group(const JoinGroupRequest request,
                        bool fire_and_forget = false)
      -> async_t<const request_result> {
    auto &groups = transition_context_.lifetime().groups();
    auto const &group_name = request.getGroup();
    spdlog::trace("cache group: {0}", group_name);
    groups.try_emplace(group_name, detail::group_context{});
    auto &group_context = groups[group_name];
    auto result = co_await send_.async_retry_send(request, &transition_context_,
                                                  fire_and_forget);
    group_context.set_joined(true);
    co_return result;
  }

  auto async_leave_group(const LeaveGroupRequest request,
                         bool fire_and_forget = false)
      -> async_t<const request_result> {
    return send_.async_retry_send(request, &transition_context_,
                                  fire_and_forget);
  }

private:
  auto
  setup_reconnect_callback(io::strand<io::io_context::executor_type> &strand) {
    auto op =
        [&ctx = transition_context_, &send_ = send_, &opt = options_](
            const detail::failed_connection_context failed_ctx) -> async_t<> {
      using to_recovering = detail::to_recovering_state;
      using to_reconnecting = detail::to_reconnecting_state;
      using to_connected_or_disconnected =
          detail::to_connected_or_disconnected_state;
      using to_disconnected = detail::to_disconnected_state;

      try {
        spdlog::trace("async_cancel_sequence_id_loop_coro beg");
        co_await send_.async_cancel_sequence_id_loop_coro();
        spdlog::trace("async_cancel_sequence_id_loop_coro end");

        // TODO: refactor this part, dont use if else
        if (failed_ctx.should_recover) {
          spdlog::trace("try.recovering... beg");

          co_await ctx.async_raise_event(to_recovering{});
          auto reconnect_url = std::move(*failed_ctx.reconnect_uri);
          auto to_final_event =
              to_connected_or_disconnected{std::move(reconnect_url)};
          co_await ctx.async_raise_event(std::move(to_final_event));
          spdlog::trace("try.recovering... end");
          auto &cur_state = ctx.get_state();
          if (std::holds_alternative<detail::connected>(cur_state)) {
            spdlog::trace("try.recovering... success");
            co_return;
          }
        }

        spdlog::trace(
            "on_receive_failed.reconnecting... beg, current state = {0}",
            ctx.get_state().index());
        if (!failed_ctx.should_recover) {
            co_await ctx.async_raise_event(to_disconnected{});
         }
        spdlog::trace("on_receive_failed.reconnecting... to reconnecting");
        co_await ctx.async_raise_event(to_reconnecting{});
        spdlog::trace("on_receive_failed.reconnecting... to "
                      "to_connected_or_disconnected_state");
        co_await ctx.async_raise_event(detail::to_connected_or_stopped_state{});
        spdlog::trace("on_receive_failed.reconnecting... end.");
      } catch (const std::exception &ex) {
        spdlog::trace(
            "unhandled exception in recovering or reconnecting, ex: {0}",
            ex.what());
        // TODO: handle ex later
        throw;
      }
    };
    auto callback = [this, &strand, op = std::move(op)](
                        const detail::failed_connection_context context) {
      io::co_spawn(strand, op(context), io::detached);
    };
    receive_.on_receive_failed.append(callback);
  }

  detail::client_lifetime_service<protocol_t, websocket_factory_t, websocket_t>
      lifetime_;
  detail::client_receive_service<protocol_t> receive_;
  detail::client_send_service<protocol_t> send_;

  detail::transition_context<detail::client_lifetime_service<
                                 protocol_t, websocket_factory_t, websocket_t>,
                             detail::client_receive_service<protocol_t>,
                             detail::client_send_service<protocol_t>>
      transition_context_;
  const detail::log log_;
  const client_options<protocol_t> &options_;
};
} // namespace webpubsub