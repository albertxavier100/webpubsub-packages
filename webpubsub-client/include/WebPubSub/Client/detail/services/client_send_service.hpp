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
#include "webpubsub/client/detail/logging/log.hpp"
#include "webpubsub/client/detail/services/client_loop_service.hpp"

// TODO: fix bug
namespace webpubsub {
namespace detail {
class client_send_service {
public:
  // TODO: test this service
  client_send_service(strand_t &strand, const log &log)
      : loop_svc_(strand, log), sequence_id_(strand) {}

  template <transition_context_c transition_context_t>
  auto spawn_sequence_ack_loop_coro(transition_context_t *context,
                                    io::cancellation_slot start_slot) {
    auto loop = [&loop_svc = loop_svc_, &sid = sequence_id_]() -> async_t<> {
      using namespace std::chrono_literals;
      for (;;) {
        try {
          uint64_t id;
          auto ok = co_await sid.async_try_get_sequence_id(id);
          if (ok) {
            // TODO: send sequence ack back to server
            spdlog::trace("send sequence ack back to server");
          }
        } catch (...) {
        }
        co_await async_delay_v2(loop_svc.strand(), 1s);
      }
    };

    loop_svc_.spawn_loop_coro(loop(), start_slot);
  }

  auto async_cancel_sequence_id_loop_coro() -> async_t<> {
    co_await loop_svc_.async_cancel_loop_coro();
    spdlog::trace("async_cancel_sequence_id_loop_coro end");
  }

private:
  client_loop_service loop_svc_;
  detail::sequence_id sequence_id_;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_SEND_SERVICE_HPP
