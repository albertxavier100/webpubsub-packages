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

class client_lifetime_service;

namespace webpubsub {
namespace detail {

class ack_cache {};

class client_receive_service {
public:
  // TODO: IMPL
  auto spawn_message_coro() {
    io::co_spawn(strand_, async_start_message_loop(), io::detached);
  }

  // TODO: IMPL
  auto cancel_message_coro() {}

  // TODO: IMPL
  auto async_wait_ack_id(uint64_t ack_id) -> async_t<> { co_return; }

private:
  auto async_start_message_loop() -> async_t<> { co_return; }

  template <typename websocket_factory_t, typename websocket_t>
    requires websocket_factory_c<websocket_factory_t, websocket_t>
  auto send_event_to_client_lifetime(
      const client_lifetime_service<websocket_factory_t, websocket_t> &lifetime,
      event_t event) -> async_t<>;
  ack_cache ack_cache_;
  strand strand_;
};

#include "webpubsub/client/detail/services/client_lifetime_service.hpp"

template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
auto send_event_to_client_lifetime(
    const client_lifetime_service<websocket_factory_t, websocket_t> &lifetime,
    event_t event) -> async_t<> {

  co_await lifetime.async_handle_event(event);
}

} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP
