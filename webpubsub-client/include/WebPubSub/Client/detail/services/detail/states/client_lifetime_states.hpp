//
// Created by alber on 2024/2/28.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_STATES_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_STATES_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/exceptions/exception.hpp"
#include "webpubsub/client/detail/services/detail/events/client_lifetime_events.hpp"
#include <variant>

// states
namespace webpubsub {
namespace detail {

template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_lifetime_service;

struct connected {};

struct connecting {
  connecting() = default;

  // TODO: IMPL
  template <typename t, typename websocket_factory_t, typename websocket_t>
    requires websocket_factory_c<websocket_factory_t, websocket_t>
  auto async_move_to(event_t ev,
                     client_lifetime_service<websocket_factory_t, websocket_t>
                         *lifetime) -> async_t<t>;

private:
  auto async_connect_websocket() -> async_t<> { co_return; }
};

struct stopped {
  stopped() = default;

  template <typename t, typename websocket_factory_t, typename websocket_t>
    requires websocket_factory_c<websocket_factory_t, websocket_t>
  auto async_move_to(event_t ev,
                     client_lifetime_service<websocket_factory_t, websocket_t>
                         *lifetime) -> async_t<t>;
};

struct recovering {};
struct stopping {};

using state_t =
    std::variant<stopped, connecting, connected, recovering, stopping>;

#include "webpubsub/client/detail/services/client_lifetime_service.hpp"
// TODO: IMPL
template <typename t, typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
auto connecting::async_move_to(
    event_t ev,
    client_lifetime_service<websocket_factory_t, websocket_t> *lifetime)
    -> async_t<t> {
  spdlog::trace("connecting -> connected: establish websocket connection");

  try {
    co_await async_connect_websocket();
    // TODO: start receive loop
    // TODO: start sequence loop
  } catch (const std::exception &ex) {
    throw invalid_operation("failed to connect to websocket");
  }

  co_return connected{};
}

template <typename t, typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
auto stopped::async_move_to(
    event_t ev,
    client_lifetime_service<websocket_factory_t, websocket_t> *lifetime)
    -> async_t<t> {
  spdlog::trace("stopped -> connecting: reset connection");
  co_return connecting{};
}

} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_STATES_HPP
