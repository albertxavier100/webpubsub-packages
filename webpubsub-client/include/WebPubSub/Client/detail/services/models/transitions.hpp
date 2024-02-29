//
// Created by alber on 2024/2/29.
//

#ifndef TEST_WEBPUBSUB_CLIENT_TRANSITIONS_HPP
#define TEST_WEBPUBSUB_CLIENT_TRANSITIONS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "client_lifetime_states.hpp"
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/concepts/websocket_factory_c.hpp"
#include "webpubsub/client/detail/async/utils.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/logging/log.hpp"
#include "webpubsub/client/detail/services/client_lifetime_service.hpp"
#include "webpubsub/client/exceptions/exception.hpp"

namespace webpubsub {
namespace detail {

#pragma region STOPPED
template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
auto async_on_event(
    client_lifetime_service<websocket_factory_t, websocket_t> *lifetime,
    stopped &stopped, to_connecting_state &event) -> async_t<state_t> {
  spdlog::trace("stopped -> connecting: reset connection");
  co_return connecting{};
}
#pragma endregion

#pragma region CONNECTING
template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
auto async_on_event(
    client_lifetime_service<websocket_factory_t, websocket_t> *lifetime,
    connecting &connecting, to_connected_state &event) -> async_t<state_t> {
  try {
    co_await lifetime->async_connect_websocket();
    auto receive_service = lifetime->get_receive_service();
    spdlog::trace(
        "connecting -> connected: receive_service->spawn_message_loop_coro");
    receive_service->spawn_message_loop_coro();
    // TODO: start sequence id loop
  } catch (const std::exception &ex) {
    throw invalid_operation("failed to connect to websocket");
  }

  co_return connected{};
}
#pragma endregion

#pragma region CONNECTED
template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
auto async_on_event(
    client_lifetime_service<websocket_factory_t, websocket_t> *lifetime,
    connected &connected, to_recovering_state &event) -> async_t<state_t> {
  spdlog::trace("connected -> recovering");
  co_return recovering{};
}
#pragma endregion

#pragma region RECOVERING
template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
auto async_on_event(
    client_lifetime_service<websocket_factory_t, websocket_t> *lifetime,
    recovering &recovering, to_connected_state &event) -> async_t<state_t> {
  spdlog::trace("recovering -> connected");
  co_return connected{};
}
#pragma endregion

#pragma region STOPPING
template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
auto async_on_event(
    client_lifetime_service<websocket_factory_t, websocket_t> *lifetime,
    stopping &stopping, to_stopped_state &event) -> async_t<state_t> {
  co_return stopped{};
}

#pragma region UNSUPPORTED
template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
auto async_on_event(
    client_lifetime_service<websocket_factory_t, websocket_t> *lifetime, auto &,
    auto &) -> async_t<state_t> {
  throw std::logic_error{"Unsupported state transition"};
}
#pragma endregion

} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_TRANSITIONS_HPP
