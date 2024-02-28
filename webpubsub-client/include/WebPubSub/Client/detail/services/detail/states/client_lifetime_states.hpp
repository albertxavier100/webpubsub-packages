//
// Created by alber on 2024/2/28.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_STATES_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_STATES_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/detail/services/detail/events/client_lifetime_events.hpp"
#include <variant>

// states
namespace webpubsub {
namespace detail {

struct connected {};

struct connecting {
  connecting() = default;

  // TODO: IMPL
  template <typename t> auto async_move_to(event_t ev) -> async_t<t> {
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

private:
  auto async_connect_websocket() -> async_t<> { co_return; }
};

struct stopped {
  stopped() = default;

  template <typename t> auto async_move_to(event_t ev) -> async_t<t> {
    spdlog::trace("stopped -> connecting: reset connection");
    co_return connecting{};
  }
};

struct recovering {};
struct stopping {};

using state_t =
    std::variant<stopped, connecting, connected, recovering, stopping>;

} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_STATES_HPP
