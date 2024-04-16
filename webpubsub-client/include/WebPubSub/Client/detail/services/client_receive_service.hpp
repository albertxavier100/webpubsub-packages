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
#include "webpubsub/client/detail/client/ack_entity.hpp"
#include "webpubsub/client/detail/client/failed_connection_context.hpp"
#include "webpubsub/client/detail/client/loop_tracker.hpp"
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
                         std::unordered_map<uint64_t, ack_entity> &ack_cache,
                         const log &log)
      : loop_svc_("RECEIVE LOOP", strand, log), ack_cache_(ack_cache),
        options_(options) {}

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
  auto
  async_start_message_loop_core(transition_context_t *context) -> async_t<> {
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
      for (auto &ack_pair : ack_cache_) {
        auto &id = ack_pair.first;
        auto &entity = ack_pair.second;
        co_await entity.async_finish_with(ack_entity::result::cancelled);
      }
      spdlog::trace("handle ack cache finished");
      auto reconnect_url = build_reconnection_url(context);

      // TODO: should also consider connection close reason
      auto should_recover = reconnect_url.has_value();
      on_receive_failed({should_recover, reconnect_url});
      spdlog::trace("fire on_receive_failed");
    }
  }

  template <transition_context_c transition_context_t>
  auto async_handle_payload(std::string payload,
                            transition_context_t *context) -> async_t<> {
    // TODO: rename the inconsistent naming
    auto response = options_.protocol.read(std::move(payload));
    if (!response) {
      spdlog::trace("failed to parse payload");
      co_return;
    }
    co_await std::visit(
        overloaded{
            [&context, this](const ConnectedResponse &res) -> async_t<> {
              return async_handle_connected_response(res, context);
            },
            [](const DisconnectedResponse &res) -> async_t<> {
              // TODO: impl
              return async_t<>();
            },
            [](const ServerMessageResponse &res) -> async_t<> {
              // TODO: impl
              return async_t<>();
            },
            [](const GroupMessageResponseV2 &res) -> async_t<> {
              // TODO: impl
              return async_t<>();
            },
            [](const AckResponse &res) -> async_t<> {
              // TODO: impl
              return async_t<>();
            },
        },
        *response);
  }

  template <transition_context_c transition_context_t>
  auto
  async_handle_connected_response(const ConnectedResponse &res,
                                  transition_context_t *context) -> async_t<> {
    context->lifetime().update_connection_info(res.moveConnectionId(),
                                               res.getReconnectionToken());
    co_await async_handle_connection_connected(res, context);
  }

  // TODO: only display on the 1st connected
  // TODO: impl
  template <transition_context_c transition_context_t>
  auto async_handle_connection_connected(const ConnectedResponse &res,
                                         transition_context_t *context)
      -> async_t<> {
    // TODO: test auto rejoin group
    if (options_.auto_rejoin_groups) {
      for (auto &pair : context->lifetime().groups()) {
        auto &name = pair.first;
        auto &group = pair.second;
        if (group.is_joined()) {
          auto request = JoinGroupRequest(name);
          auto result = co_await context->send().async_retry_send(
              request, context, false);
          try {
          } catch (const std::exception &ex) {
          }
        }
      }
    }

    safe_invoke_connected_callback(res, context);
  }

  template <transition_context_c transition_context_t>
  auto safe_invoke_connected_callback(const ConnectedResponse &res,
                                      transition_context_t *context) {
    context->on_connected(connected_context{
        res.moveConnectionId(), res.moveUserId(), res.moveReconnectionToken()});
  }

  auto
  concat_query(const std::map<std::string, std::string> &query) -> std::string {
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

  const client_options<protocol_t> &options_;
  client_loop_service loop_svc_;
  std::unordered_map<uint64_t, detail::ack_entity> &ack_cache_;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP
