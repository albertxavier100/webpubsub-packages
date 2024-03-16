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
#include "webpubsub/client/detail/client/sequence_id.hpp"
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
  client_receive_service(strand_t &strand, const log &log)
      : loop_svc_(strand, log), loop_tracker_(strand) {}

  eventpp::CallbackList<void(const bool)> on_receive_failed;

  template <transition_context_c transition_context_t>
  auto spawn_message_loop_coro(transition_context_t &context,
                               io::cancellation_slot start_slot) {
    loop_svc_.spawn_loop_coro(async_start_message_loop(context),
                              std::move(start_slot));
  }

  auto async_cancel_message_loop_coro() -> async_t<> {
    co_await loop_svc_.async_cancel_loop_coro();
    spdlog::trace("wait loop finish begin");
    co_await loop_tracker_.async_wait();
    spdlog::trace("wait loop finish end");
    co_return;
  }

  // TODO: use context pointer
  template <transition_context_c transition_context_t>
  auto async_start_message_loop(transition_context_t &context) -> async_t<> {
    struct exit_scope {
      ~exit_scope() {
        spdlog::trace("receive loop finished beg");
        lt_.finish();
      }
      loop_tracker &lt_;
    } _{loop_tracker_};

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
      // TODO: handle unhandled ack entity
    }

    if (!ok) {
      on_receive_failed(true);
    }
    co_return;
  }

private:
  client_loop_service loop_svc_;
  // TODO: rename
  loop_tracker loop_tracker_;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP
