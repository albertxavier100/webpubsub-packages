//
// Created by alber on 2024/2/27.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_events.hpp"

namespace webpubsub {
namespace detail {

class ack_cache {};

template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_lifetime_service;

template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_receive_service {

  using client_channel_service_t = detail::client_channel_service;

public:
  client_receive_service(strand &strand,
                         const client_channel_service_t &channel_service,
                         const log &log)
      : strand_(strand), log_(log), channel_service_(channel_service) {}

  // TODO: IMPL
  auto spawn_message_loop_coro() {
    spdlog::trace("client_receive_service.spawn_message_loop_coro");
    io::co_spawn(strand_, async_start_message_loop(), io::detached);
  }

  // TODO: IMPL
  auto cancel_message_loop_coro() {}

  // TODO: IMPL
  auto async_wait_ack_id(uint64_t ack_id) -> async_t<> { co_return; }

  auto
  set_lifetime_service(client_lifetime_service<websocket_factory_t, websocket_t>
                           *lifetime_service);

private:
  auto async_start_message_loop() -> async_t<> {
    using namespace std::chrono_literals;
    spdlog::trace("client_receive_service.async_start_message_loop begin");
    bool should_recover = false;
    try {

      for (;;) {
        co_await async_delay_v2(strand_, 1s);
        // TODO: debug
        throw std::exception("dev");
      }
    } catch (const std::exception &ex) {
      should_recover = true;
    }

    if (should_recover) {
      spdlog::trace("async_start_message_loop -- "
                    "lifetime_service_->async_raise_event -- begin");
      co_await lifetime_service_->async_raise_event(to_recovering_state{});
    }

    co_return;
  }

  ack_cache ack_cache_;
  strand &strand_;
  const client_channel_service_t &channel_service_;
  const log &log_;
  client_lifetime_service<websocket_factory_t, websocket_t> *lifetime_service_;
};

#pragma region IMPL
#include "webpubsub/client/detail/services/client_lifetime_service.hpp"
template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
auto client_receive_service<websocket_factory_t, websocket_t>::
    set_lifetime_service(
        client_lifetime_service<websocket_factory_t, websocket_t>
            *lifetime_service) {
  lifetime_service_ = lifetime_service;
}
#pragma endregion
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP
