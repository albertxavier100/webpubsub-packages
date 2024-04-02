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
#include "webpubsub/client/detail/concepts/client_lifetime_service_c.hpp"
#include "webpubsub/client/detail/concepts/transition_context_c.hpp"
#include "webpubsub/client/detail/services/client_loop_service.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_events.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_states.hpp"

namespace webpubsub {
namespace detail {
class client_receive_service {
public:
  client_receive_service(strand_t &strand,
                         std::unordered_map<uint64_t, ack_entity> &ack_cache,
                         const log &log)
      : loop_svc_(strand, log), loop_tracker_(strand), ack_cache_(ack_cache) {}

  eventpp::CallbackList<void(const bool)> on_receive_failed;

  template <transition_context_c transition_context_t>
  auto spawn_message_loop_coro(transition_context_t &context,
                               io::cancellation_slot start_slot) {
    loop_svc_.spawn_loop_coro(async_start_message_loop(context),
                              std::move(start_slot));
  }

  auto async_cancel_message_loop_coro() -> async_t<> {
    co_await loop_svc_.async_cancel_loop_coro();
  }

  template <transition_context_c transition_context_t>
  auto async_start_message_loop(transition_context_t &context) -> async_t<> {
    co_await loop_svc_.async_start_loop(async_start_message_loop_core(context));
  }

  template <transition_context_c transition_context_t>
  auto
  async_start_message_loop_core(transition_context_t &context) -> async_t<> {
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
        co_await context.lifetime().async_read_message(payload, status);
        // TODO: handle message
        spdlog::trace("receiving...");
      }
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
      // TODO: decide should recover or reconnect, and log here
      on_receive_failed(false);
    }
  }

private:
  client_loop_service loop_svc_;
  // TODO: rename
  loop_tracker loop_tracker_;
  std::unordered_map<uint64_t, detail::ack_entity> &ack_cache_;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP
