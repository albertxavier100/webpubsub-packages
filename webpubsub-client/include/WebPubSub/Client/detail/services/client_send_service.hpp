//
// Created by alber on 2024/2/27.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_SEND_SERVICE_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_SEND_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/detail/services/client_loop_service.hpp"
#include "webpubsub/client/detail/client/sequence_id.hpp"
#include "webpubsub/client/detail/concepts/transition_context_c.hpp"
#include "webpubsub/client/models/request_result.hpp"

namespace webpubsub {
namespace detail {

template <webpubsub_protocol_t protocol_t> class client_send_service {
public:
  // TODO: low: test this service
  client_send_service(strand_t &strand,
                      const client_options<protocol_t> &options, const log &log)
      : loop_svc_("SEQUENCE LOOP", strand, log), sequence_id_(strand),
        options_(options) {}

template <transition_context_c transition_context_t>
  auto spawn_sequence_ack_loop_coro(transition_context_t *context) {
    loop_svc_.spawn_loop_coro(async_start_sequence_ack_loop(context));
  }

  auto async_cancel_sequence_id_loop_coro() -> async_t<> {
    spdlog::trace("async_cancel_sequence_id_loop_coro beg");
    co_await loop_svc_.async_cancel_loop_coro();
    spdlog::trace("async_cancel_sequence_id_loop_coro end");
  }

  auto reset() -> void {
    sequence_id_.reset();
    loop_svc_.reset();
  }

  // TODO: [HIGH] add result
  template <typename request_t, transition_context_c transition_context_t>
  auto async_send_request(request_t request,
                          transition_context_t *context) -> async_t<> {
    const auto &protocol = options_.protocol;
    try {
      auto frame = protocol.write(request);
      co_await context->lifetime().async_write_message(frame);
    } catch (const std::exception &ex) {
      spdlog::trace("Failed to send message. ex: {0}", ex.what());
    }
  }

  template <typename request_t, transition_context_c transition_context_t>
  auto
  async_retry_send(request_t request, transition_context_t *context,
                   bool fire_and_forget = false) -> async_t<const request_result> {
    auto retry_options = options_.message_retry_options;
    if (!request.hasAckId()) {
      auto ack_id = context->next_ack_id();
      request.setAckId(ack_id);
    }
    retry_context retry_context{
        retry_options.max_delay, retry_options.max_retry,
        retry_options.retry_mode, 0, retry_options.delay};
    for (;;) {
      try {
        if (fire_and_forget) {
          co_await async_send_request(std::move(request), context);
        } else {
          auto &cache = context->ack_cache();
          auto id = *request.getAckId();
          cache.add_or_get(context->strand(), id);
          co_await async_send_request(std::move(request), context);
          co_await cache.async_wait(id);
        }

        // TODO: [HIGH] impl
        co_return request_result{};
      } catch (const std::exception &ex) {
        spdlog::trace("send message failed in {0} times",
                      retry_context.attempts);
      }

      update_delay(retry_context);
      if (!retry_context.delay) {
        break;
      }
      co_await async_delay_v2(context->strand(), *retry_context.delay);
    }
    spdlog::trace("send message retry failed");
    // TODO: [HIGH] impl
    co_return request_result{};
  }

  auto sequence_id() -> sequence_id & { return sequence_id_; }

private:
  template <transition_context_c transition_context_t>
  auto async_start_sequence_ack_loop(transition_context_t *context)
      -> async_t<> {
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
          co_await async_send_request(SequenceAckSignal{id}, context);
          spdlog::trace("send sequence ack: {0} back to server...", id);
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
  // TODO: low: move to transition context
  detail::sequence_id sequence_id_;
  const client_options<protocol_t> &options_;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_SEND_SERVICE_HPP
