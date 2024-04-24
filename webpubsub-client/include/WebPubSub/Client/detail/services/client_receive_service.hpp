//
// Created by alber on 2024/2/27.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "uri.hh"
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/client/ack_cache.hpp"
#include "webpubsub/client/detail/client/failed_connection_context.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/common/utils.hpp"
#include "webpubsub/client/detail/concepts/client_lifetime_service_c.hpp"
#include "webpubsub/client/detail/concepts/transition_context_c.hpp"
#include "webpubsub/client/detail/services/client_loop_service.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_events.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_states.hpp"
#include "webpubsub/client/models/client_options.hpp"

namespace webpubsub {
namespace detail {

template <webpubsub_protocol_t protocol_t> class client_receive_service {
public:
  client_receive_service(strand_t &strand,
                         const client_options<protocol_t> &options,
                         const log &log)
      : loop_svc_("RECEIVE LOOP", strand, log), options_(options),
        server_data_channel_(strand, options.max_buffer_size),
        group_data_channel_(strand, options.max_buffer_size) {}

  eventpp::CallbackList<void(const failed_connection_context)>
      on_receive_failed;

  template <transition_context_c transition_context_t>
  auto spawn_message_loop_coro(transition_context_t *context) {
    loop_svc_.spawn_loop_coro(async_start_message_loop_core(context));
  }

  auto async_cancel_message_loop_coro() -> async_t<> {
    spdlog::trace("async_cancel_message_loop_coro begin");
    co_await loop_svc_.async_cancel_loop_coro();
    spdlog::trace("async_cancel_message_loop_coro end");
  }

  auto reset() -> void { loop_svc_.reset(); }

private:
  template <transition_context_c transition_context_t>
  auto async_start_message_loop_core(transition_context_t *context)
      -> async_t<> {
    using namespace std::chrono_literals;
    spdlog::trace("client_receive_service.async_start_message_loop begin");
    bool ok = true;
    try {
      for (;;) {
        auto cs = co_await io::this_coro::cancellation_state;
        if (cs.cancelled() != io::cancellation_type::none) {
          spdlog::trace("receiving... break");
          break;
        }
        std::string payload;
        websocket_close_status status;
        co_await context->lifetime().async_read_message(payload, status);

        async_handle_payload(std::move(payload), context);
        spdlog::trace("receiving...");
      }
    } catch (const io::system_error &err) {
      spdlog::trace("message loop stopped with system error: {0}", err.what());
    } catch (const std::exception &ex) {
      spdlog::trace("message loop stopped with ex: {0}", ex.what());
      ok = false;
    }

    if (!ok) {
      spdlog::trace("on_receive_failed");
      context->ack_cache().finish_all(ack_cache::result::cancelled);
      spdlog::trace("handle ack cache finished");
      auto reconnect_url = build_reconnection_url(context);

      // TODO: should also consider connection close reason
      auto should_recover = reconnect_url.has_value();
      on_receive_failed({should_recover, reconnect_url});
      spdlog::trace("fire on_receive_failed");
    }
  }

  template <transition_context_c transition_context_t>
  auto async_handle_payload(std::string &&payload,
                            transition_context_t *context) -> async_t<> {
    // TODO: low: rename the inconsistent naming in protocol
    auto response = options_.protocol.read(payload);
    if (!response) {
      spdlog::trace("failed to parse payload");
      co_return;
    }

    co_await std::visit(
        [&context, this](auto &&res) -> async_t<> {
          using T = std::decay_t<decltype(res)>;
          if constexpr (std::is_same_v<T, ConnectedResponse>) {
            return async_handle_connected_response(std::move(res), context);
          } else if constexpr (std::is_same_v<T, DisconnectedResponse>) {
            return async_handle_disconnected_response(std::move(res), context);
          } else if constexpr (std::is_same_v<T, GroupMessageResponseV2>) {
            return async_handle_group_data_response(std::move(res), context);
          } else if constexpr (std::is_same_v<T, ServerMessageResponse>) {
            return async_handle_server_data_response(std::move(res), context);
          } else if constexpr (std::is_same_v<T, AckResponse>) {
            return async_handle_ack_response(std::move(res), context);
          } else {
            static_assert(false, "unsupported response");
          }
          return async_t<>();
        },
        std::move(*response));
  }

  template <transition_context_c transition_context_t>
  auto check_state(transition_context_t *context)
      -> std::optional<const connected *> {
    if (const auto connected_state =
            std::get_if<connected>(&(context->get_state()))) {
      return connected_state;
    }
    spdlog::trace("client is not in connected state");
    return std::nullopt;
  }

  template <transition_context_c transition_context_t>
  auto async_handle_ack_response(AckResponse &&res,
                                 transition_context_t *context) -> async_t<> {
    ack_cache::result_t result;
    // TODO: use a const string for error name
    if (res.getSuccess() ||
        res.getError() && res.getError().value().getName() == "Duplicate") {
      result = ack_cache::result::completed;
    } else {
      result = invalid_operation(
          "Received non-success acknowledge from the service");
    }
    // TODO: use wps result instead
    context->ack_cache().finish(res.getAckId(), std::move(result));
    return async_t<>();
  }

  template <transition_context_c transition_context_t>
  auto async_handle_disconnected_response(DisconnectedResponse &&res,
                                          transition_context_t *context)
      -> async_t<> {
    if (auto connected_state =
            std::get_if<connected>(&(context->get_state()))) {
      connected_state->disconnected_message = *res.moveMessage();
    } else {
      spdlog::trace("client is not in connected state");
    }
    return async_t<>();
  }

  template <transition_context_c transition_context_t>
  auto async_handle_server_data_response(ServerMessageResponse &&res,
                                         transition_context_t *context)
      -> async_t<> {
    if (res.getSequenceId()) {
      auto &sid = context->send().sequence_id();
      if (!(co_await sid.async_try_update(*res.getSequenceId()))) {
        co_return;
      }
    }
    co_await server_data_channel_.async_send(io::error_code{}, res,
                                             io::use_awaitable);
  }

  template <transition_context_c transition_context_t>
  auto async_handle_group_data_response(GroupMessageResponseV2 &&res,
                                        transition_context_t *context)
      -> async_t<> {
    if (res.getSequenceId()) {
      auto &sid = context->send().sequence_id();
      if (!(co_await sid.async_try_update(*res.getSequenceId()))) {
        co_return;
      }
    }
    co_await group_data_channel_.async_send(io::error_code{}, res,
                                            io::use_awaitable);
  }

  template <transition_context_c transition_context_t>
  auto async_handle_connected_response(ConnectedResponse &&res,
                                       transition_context_t *context)
      -> async_t<> {
    const auto connected_state = check_state(context);
    if (connected_state) {
      context->lifetime().update_connection_info(res.getConnectionId(),
                                                 res.getReconnectionToken());
      if ((*connected_state)->first_connected) {
        return async_handle_connection_connected(res, context);
      }
    }
    return async_t<>();
  }

  template <transition_context_c transition_context_t>
  auto async_handle_connection_connected(const ConnectedResponse &res,
                                         transition_context_t *context)
      -> async_t<> {
    // TODO: low: test auto rejoin group
    if (options_.auto_rejoin_groups) {
      for (auto &pair : context->lifetime().groups()) {
        auto &name = pair.first;
        auto &group = pair.second;
        if (group.is_joined()) {
          auto request = JoinGroupRequest(name);
          std::optional<std::exception> ex;
          try {
            auto result = co_await context->send().async_retry_send(
                request, context, false);
          } catch (const std::exception &e) {
            ex = e;
          }
          if (ex) {
            rejoin_group_failed_context callback_context{name, *ex};
            context->safe_invoke_callback(std::move(callback_context));
          }
        }
      }
    }
    connected_context callback_context{res.moveConnectionId(), res.moveUserId(),
                                       res.moveReconnectionToken()};
    context->safe_invoke_callback(std::move(callback_context));
  }

  auto async_start_handle_group_data_response() -> async_t<> {}

  auto concat_query(const std::map<std::string, std::string> &query)
      -> std::string {
    bool first = true;
    std::stringstream ss;
    for (const auto &kv : query) {
      auto de = first ? "" : "&";
      first = false;
      ss << de << kv.first << "=" << kv.second;
    }
    return ss.str();
  }

  template <transition_context_c transition_context_t>
  auto build_reconnection_url(transition_context_t *context)
      -> std::optional<std::string> {
    auto &lifetime = context->lifetime();
    const auto &id = lifetime.connection_id();
    const auto &token = lifetime.reconnection_token();
    if (id.empty() || !token) {
      return std::nullopt;
    }

    uri client_access_uri(lifetime.client_access_uri());
    auto query_map = client_access_uri.get_query_dictionary();
    query_map[lifetime.RECOVER_CONNECTION_ID_QUERY] = id;
    query_map[lifetime.RECOVER_RECONNECTION_TOKEN_QUERY] = *token;
    auto query_str = concat_query(query_map);
    std::map<uri::component, std::string> replacement{
        {uri::component::Query, std::move(query_str)}};
    uri client_reconnect_uri(client_access_uri, replacement);
    auto uri_str = client_reconnect_uri.to_string();
    return std::move(uri_str);
  }

  template <transition_context_c transition_context_t>
  auto async_start_group_data_response_handler(transition_context_t *context)
      -> async_t<> {
    for (;;) {
      auto [ec, res] =
          co_await group_data_channel_.async_receive(io::use_awaitable);
      group_data_context callback_context{
          res.getGroup(), res.getSequenceId(), res.getFromUseId(),
          res.getDataType()
          // TODO: data
      };
      context->safe_invoke_callback(callback_context);
    }
  }

  template <transition_context_c transition_context_t>
  auto async_start_server_data_response_handler(transition_context_t *context)
      -> async_t<> {
    for (;;) {
      auto [ec, res] =
          co_await server_data_channel_.async_receive(io::use_awaitable);
      server_data_context callback_context{
          res.getSequenceId(), res.getDataType(),
          // TODO: data
      };
      context->safe_invoke_callback(callback_context);
    }
  }
  const client_options<protocol_t> &options_;
  client_loop_service loop_svc_;
  io::experimental::channel<void(io::error_code, ServerMessageResponse)>
      server_data_channel_;
  io::experimental::channel<void(io::error_code, GroupMessageResponseV2)>
      group_data_channel_;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP
