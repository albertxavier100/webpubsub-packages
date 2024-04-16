//
// Created by alber on 2024/2/27.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_SEND_SERVICE_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_SEND_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/client/sequence_id.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/concepts/transition_context_c.hpp"
#include "webpubsub/client/detail/logging/log.hpp"
#include "webpubsub/client/detail/services/client_loop_service.hpp"
#include "webpubsub/client/models/request_result.hpp"

namespace webpubsub {
namespace detail {

template <webpubsub_protocol_t protocol_t> class client_send_service {
public:
  // TODO: test this service
  client_send_service(strand_t &strand,
                      const client_options<protocol_t> &options, const log &log)
      : loop_svc_("SEQUENCE LOOP", strand, log), sequence_id_(strand),
        options_(options) {
    // TODO: improve this
    const auto &max_retry = options.reconnect_retry_options.max_retry;
    const auto &max_delay = options.reconnect_retry_options.max_delay;
    const auto &delay = options.reconnect_retry_options.delay;
    switch (options.reconnect_retry_options.retry_mode) {
    case retry_mode::exponential: {
      retry_policy_.emplace<exponential_retry_policy>(
          exponential_retry_policy(max_retry, delay, max_delay));
      return;
    }
    case retry_mode::fixed: {
      retry_policy_.emplace<fixed_retry_policy>(
          fixed_retry_policy(max_retry, delay));
      return;
    }
    }
  }

  auto spawn_sequence_ack_loop_coro() {
    loop_svc_.spawn_loop_coro(async_start_sequence_ack_loop());
  }

  auto async_cancel_sequence_id_loop_coro() -> async_t<> {
    spdlog::trace("async_cancel_sequence_id_loop_coro beg");
    co_await loop_svc_.async_cancel_loop_coro();
    spdlog::trace("async_cancel_sequence_id_loop_coro end");
  }

  auto reset() -> void {
    sequence_id_.reset();
    loop_svc_.reset();
    std::visit(overloaded{[](auto &p) { p.reset(); }}, retry_policy_);
  }

  template <typename message_t, transition_context_c transition_context_t>
  auto async_send_message(const message_t message,
                          transition_context_t context) {
    const auto &protocol = options_.protocol;
    try {
      auto frame = protocol.write(message);
      co_await context->lifetime().async_write_message(frame);
    } catch (const std::exception &ex) {
      spdlog::trace("Failed to send message. ex: {0}", ex.what());
    }
  }

  template <typename message_t, transition_context_c transition_context_t>
  auto
  async_retry_send(message_t message, transition_context_t context,
                   bool fire_and_forget = false) -> async_t<request_result> {
    for (auto attempt = 0;; attempt++) {
      try {
        co_await async_send_message(std::move(message), context);
        // TODO: wait ack?
        // TODO: impl
        co_return request_result{};
      } catch (const std::exception &ex) {
        spdlog::trace("send message failed in {0} times", attempt);
      }

      auto delay = std::visit(
          overloaded{[](auto &policy) { return policy.next_retry_delay(); }},
          retry_policy_);
      if (!delay) {
        break;
      }
      co_await async_delay_v2(context->strand(), *delay);
    }
    spdlog::trace("send message retry failed");
    // TODO: impl
    co_return request_result{};
  }

private:
  auto async_start_sequence_ack_loop() -> async_t<> {
    using namespace std::chrono_literals;
    spdlog::trace("async_start_sequence_ack_loop beg");

    for (;;) {
      spdlog::trace("async_start_sequence_ack_loop in loop");
      auto cs = co_await io::this_coro::cancellation_state;
      if (cs.cancelled() != io::cancellation_type::none) {
        spdlog::trace("sequence ack... break");
        break;
      }
      try {
        uint64_t id;
        spdlog::trace("async_try_get_sequence_id beg");
        auto ok = co_await sequence_id_.async_try_get_sequence_id(id);
        spdlog::trace("async_try_get_sequence_id end");
        if (ok) {
          // TODO: send sequence ack back to server
          spdlog::trace("send sequence ack back to server...");
        }
        spdlog::trace("sequence ack in loop...");
      } catch (const io::system_error &err) {
        spdlog::trace("send sequence ack got system error: {0}", err.what());
        break;
      } catch (const std::exception &ex) {
        spdlog::trace("send sequence ack got exception: {0}", ex.what());
      }
      co_await async_delay_v2(loop_svc_.strand(), 1s);
    }

    co_return;
  };

  client_loop_service loop_svc_;
  detail::sequence_id sequence_id_;
  const client_options<protocol_t> &options_;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_SEND_SERVICE_HPP
