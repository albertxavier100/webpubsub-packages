#pragma once

#include "eventpp/callbacklist.h"
#include "webpubsub/client/concepts/websocket_factory_c.hpp"
#include "webpubsub/client/detail/async/exclusion_lock.hpp"
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
class client_v2 {
  template <typename t = void> using async_t = io::awaitable<t>;

public:
  client_v2(io::strand<io::io_context::executor_type> &strand,
            const client_options<protocol_t> &options,
            websocket_factory_t &websocket_factory,
            const std::string &logger_name)
      : log_(logger_name), ack_cache_(),
        lifetime_(strand, websocket_factory, options, log_),
        receive_(strand, ack_cache_, log_), send_(strand, log_),
        transition_context_(strand, lifetime_, receive_, send_, log_),
        on_connected(transition_context_.on_connected),
        on_disconnected(transition_context_.on_disconnected),
        on_group_data(transition_context_.on_group_data),
        on_server_data(transition_context_.on_server_data),
        on_rejoin_group_failed(transition_context_.on_rejoin_group_failed),
        on_stopped(transition_context_.on_stopped) {
    setup_reconnect_callback(strand);
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
    // TODO: use real string
    auto event = detail::to_connected_state{"TODO", "TODO", "TODO"};
    co_await transition_context_.async_raise_event(std::move(event));
  }

  auto async_stop() -> async_t<> {
    spdlog::trace("async_stop -- beg");
    co_await transition_context_.async_raise_event(detail::to_stopping_state{});
    co_await transition_context_.async_raise_event(detail::to_stopped_state{});
    spdlog::trace("async_stop -- end");
  }

  // TODO: test
  auto async_cancel() -> async_t<> {
    // transition_context_.cancel_signal.emit(cancel_type);
    co_await send_.async_cancel_sequence_id_loop_coro();
    co_await receive_.async_cancel_message_loop_coro();
  }

private:
  auto
  setup_reconnect_callback(io::strand<io::io_context::executor_type> &strand) {
    auto &ctx = transition_context_;
    auto op = [&ctx, &send_ = this->send_](const bool recover) -> async_t<> {
      using to_recovering = detail::to_recovering_state;
      using to_reconnecting = detail::to_reconnecting_state;
      using to_connected_or_disconnected =
          detail::to_connected_or_disconnected_state;
      using to_disconnected = detail::to_disconnected_state;

      try {
        spdlog::trace("async_cancel_sequence_id_loop_coro beg");
        co_await send_.async_cancel_sequence_id_loop_coro();
        spdlog::trace("async_cancel_sequence_id_loop_coro end");

        if (recover) {
          spdlog::trace("try.recovering... beg");
          co_await ctx.async_raise_event(to_recovering{});
          // TODO: add close status
          co_await ctx.async_raise_event(to_connected_or_disconnected{});
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
        if (!recover) {
          // TODO: use actual string
          co_await ctx.async_raise_event(to_disconnected{"TODO", "TODO"});
        }
        spdlog::trace("on_receive_failed.reconnecting... to reconnecting");
        co_await ctx.async_raise_event(to_reconnecting{});
        spdlog::trace("on_receive_failed.reconnecting... to "
                      "to_connected_or_disconnected_state");
        co_await ctx.async_raise_event(to_connected_or_disconnected{});
        spdlog::trace("on_receive_failed.reconnecting... end");
      } catch (const std::exception &ex) {
        spdlog::trace("failed to recover, ex: {0}", ex.what());
        throw;
      }
    };
    auto callback = [this, &strand, op = std::move(op)](const bool recover) {
      io::co_spawn(strand, op(recover), io::detached);
    };
    receive_.on_receive_failed.append(callback);
  }

  detail::client_lifetime_service<protocol_t, websocket_factory_t, websocket_t>
      lifetime_;
  detail::client_receive_service receive_;
  detail::client_send_service send_;

  detail::transition_context<detail::client_lifetime_service<
                                 protocol_t, websocket_factory_t, websocket_t>,
                             detail::client_receive_service,
                             detail::client_send_service>
      transition_context_;
  const detail::log log_;
  // TODO: move to receive service
  std::unordered_map<uint64_t, detail::ack_entity> ack_cache_;
};
} // namespace webpubsub