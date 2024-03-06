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
#include "webpubsub/client/detail/client/sequence_id.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/services/client_loop_service.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_events.hpp"

namespace webpubsub {
namespace detail {

template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_lifetime_service;

template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_receive_service {
public:
  client_receive_service(strand_t &strand, const log &log)
      : loop_svc_(strand, log) {}

  auto
  async_spawn_message_loop_coro(io::cancellation_slot start_slot) -> async_t<> {
    co_await loop_svc_.async_spawn_loop_coro(async_start_message_loop(),
                                             std::move(start_slot));
  }

  auto async_cancel_message_loop_coro() -> async_t<> {
    co_await loop_svc_.async_cancel_loop_coro();
    spdlog::trace("message loop cancel waited");
    co_return;
  }

  auto
  set_lifetime_service(client_lifetime_service<websocket_factory_t, websocket_t>
                           *lifetime_service) {
    loop_svc_.set_lifetime_service(lifetime_service);
  }

private:
  auto async_start_message_loop() -> async_t<> {
    using namespace std::chrono_literals;
    spdlog::trace("client_receive_service.async_start_message_loop begin");
    bool should_recover = false;
    try {
      for (;;) {
        auto cs = co_await io::this_coro::cancellation_state;
        if (cs.cancelled() != io::cancellation_type::none) {
          spdlog::trace("receiving... break");
          break;
        }
        co_await async_delay_v2(loop_svc_.strand(), 1s);
        spdlog::trace("receiving...");
      }
    } catch (const std::exception &ex) {
      spdlog::trace("message loop stopped with ex: {0}", ex.what());
      should_recover = true;
    }

    if (should_recover) {
      spdlog::trace("async_start_message_loop -- "
                    "lifetime_->async_raise_event -- begin");
      co_await loop_svc_.lifetime()->async_raise_event(to_recovering_state{});
      co_await loop_svc_.lifetime()->async_raise_event(to_connected_state{});
    }
    co_return;
  }

  client_loop_service<websocket_factory_t, websocket_t> loop_svc_;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP
