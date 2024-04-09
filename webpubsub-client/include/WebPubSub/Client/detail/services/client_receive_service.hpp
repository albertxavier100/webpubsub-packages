//
// Created by alber on 2024/2/27.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/client/ack_entity.hpp"
#include "webpubsub/client/detail/client/loop_tracker.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/common/utils.hpp"
#include "webpubsub/client/detail/concepts/client_lifetime_service_c.hpp"
#include "webpubsub/client/detail/concepts/transition_context_c.hpp"
#include "webpubsub/client/detail/services/client_loop_service.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_events.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_states.hpp"

namespace webpubsub {
namespace detail {
template <webpubsub_protocol_t protocol_t> class client_receive_service {
public:
  client_receive_service(strand_t &strand, const protocol_t &protocol,
                         std::unordered_map<uint64_t, ack_entity> &ack_cache,
                         const log &log)
      : loop_svc_("RECEIVE LOOP", strand, log), ack_cache_(ack_cache),
        protocol_(protocol) {}

  eventpp::CallbackList<void(const bool)> on_receive_failed;

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
        handle_payload(std::move(payload));
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
      // TODO: decide should recover or reconnect, and log here
      // TODO: use different callback, do not use branch
      on_receive_failed(false);
      spdlog::trace("fire on_receive_failed");
    }
  }

  auto handle_payload(std::string payload) {
    // TODO: rename the inconsistent naming
    auto response = protocol_.read(std::move(payload));
    std::visit(overloaded{
        [&response](ConnectedResponse &res) {

        },
        [&response](DisconnectedResponse &res) {

        },
        [&response](ServerMessageResponse &res) {

        },
        [&response](GroupMessageResponseV2 &res) {

        },
        [&response](AckResponse &res) {

        },
    });
  }

  protocol_t &protocol_;
  client_loop_service loop_svc_;
  std::unordered_map<uint64_t, detail::ack_entity> &ack_cache_;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP
