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
#include "webpubsub/client/detail/concepts/transition_context_c.hpp"
#include "webpubsub/client/detail/logging/log.hpp"
#include "webpubsub/client/detail/services/client_lifetime_service.hpp"
#include "webpubsub/client/exceptions/exception.hpp"

namespace webpubsub {
namespace detail {
#pragma region STOPPED
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, stopped &stopped,
                    to_connecting_state &event) -> async_t<state_t> {
  spdlog::trace("stopped -> connecting: reset connection");
  // TODO: reset connection
  co_return connecting{};
}
#pragma endregion

#pragma region CONNECTING
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, connecting &connecting,
                    to_connected_state &event) -> async_t<state_t> {
  try {

    co_await context->lifetime().async_connect_websocket();
    spdlog::trace("connecting -> connected: rcv->spawn_message_loop_coro");
    context->receive().spawn_message_loop_coro(*context,
                                               std::move(event.start_slot));
    // TODO: start sequence id loop
  } catch (const std::exception &ex) {
    throw invalid_operation("failed to connect to websocket");
  }

  co_return connected{};
}
#pragma endregion

#pragma region CONNECTED
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, connected &connected,
                    to_recovering_state &event) -> async_t<state_t> {
  // TODO: reset connection
  spdlog::trace("connected -> recovering");
  co_return recovering{};
}

template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, connected &connected,
                    to_stopping_state &event) -> async_t<state_t> {
  spdlog::trace("connected -> stopping");
  co_return stopping{};
}
#pragma endregion

#pragma region RECOVERING
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, recovering &recovering,
                    to_connected_state &event) -> async_t<state_t> {
  spdlog::trace("recovering -> connected");
  try {
    // TODO: reconnect or stop
    co_return connected{};
  } catch (...) {
    co_return disconnected{};
  }
}

#pragma endregion

#pragma region STOPPING
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, stopping &stopping,
                    to_stopped_state &event) -> async_t<state_t> {
  // TODO: cancel receive loop
  co_await context->receive().async_cancel_message_loop_coro();
  // TODO: cancel sequence loop
  spdlog::trace("stopping -> stopped");
  co_return stopped{};
}

#pragma region UNSUPPORTED
// TODO: careful about un-wired transitions
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, auto &state,
                    auto &event) -> async_t<state_t> {
  throw std::logic_error{
      std::format("Unsupported state and transition: state: {}, transition: {}",
                  typeid(state).name(), typeid(event).name())};
}
#pragma endregion

} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_TRANSITIONS_HPP
