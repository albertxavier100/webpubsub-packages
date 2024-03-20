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
// stopped -> connecting
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, stopped &stopped,
                    to_connecting_state &event) -> async_t<state_t> {
  spdlog::trace(":::Transition::: stopped -> connecting: reset connection");
  // TODO: reset connection
  co_return connecting{};
}

// enter stopped
template <transition_context_c transition_context_t>
auto async_on_enter(transition_context_t *context, stopped &stopped,
                    to_stopped_state &event) -> async_t<> {
  spdlog::trace(":::Transition::: enter stopped state");
  spdlog::trace("notify stopped callback");
}

#pragma endregion

#pragma region CONNECTING
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, connecting &connecting,
                    to_connected_state &event) -> async_t<state_t> {
  spdlog::trace(":::Transition::: connecting -> connected ");
  try {
    co_await context->lifetime().async_connect_new_websocket();
    co_return connected{};
  } catch (const std::exception &ex) {
    // TODO: reconnect
    spdlog::trace("failed to connect to websocket");
    throw invalid_operation("failed to connect to websocket");
  }
}
#pragma endregion

#pragma region CONNECTED
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, connected &connected,
                    to_disconnected_state &event) -> async_t<state_t> {
  // TODO: reset connection
  spdlog::trace(":::Transition::: connected -> disconnected");
  try {
    context->on_disconnected(disconnected_context{
        .connection_id = std::move(event.connection_id),
        .reason = std::move(event.reason),
    });
  } catch (const std::exception &ex) {
    spdlog::trace(":::Transition::: failed to invoke disconnected event: {0}",
                  ex.what());
  }
  co_return disconnected{};
}

template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, connected &connected,
                    to_stopping_state &event) -> async_t<state_t> {
  spdlog::trace(":::Transition::: connected -> stopping");
  co_return stopping{};
}
#pragma endregion

#pragma region DISCONNECTED
// disconnected => recovering or stopped
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, disconnected &disconnected,
                    to_recovering_or_stopped_state &event) -> async_t<state_t> {
  spdlog::trace(":::Transition::: disconnected -> recovering / stopped");

  // TODO: finish any awaiting ack entity
  if (context->lifetime().auto_reconnect()) {
    co_return recovering{};
  }
  try {
    context->on_stopped(stopped_context{});
  } catch (const std::exception &ex) {
    spdlog::trace("failed to invoke disconnected event: {0}", ex.what());
  }
  co_return stopped{};
}

// enter disconnected state
template <transition_context_c transition_context_t>
auto async_on_enter(transition_context_t *context, disconnected &disconnected,
                    to_disconnected_state &event) -> async_t<state_t> {
  try {
    context->on_disconnected(disconnected_context{
        .connection_id = event.connection_id, .reason = event.reason});
  } catch (const std::exception &ex) {
  }
}
#pragma endregion

#pragma region RECOVERING

template <transition_context_c transition_context_t>
auto async_reconnect(transition_context_t *context) -> async_t<> {
  auto retry_policy = context->lifetime.retry_policy();
  for (;;) {
    try {
      co_await context->lifetime().async_connect_new_websocket();
    } catch (const std::exception &ex) {
      spdlog::trace("failed to reconnect. {0}", ex.what());
    }
    auto delay = retry_policy.next_retry_delay();
    if (!delay) {
      co_return stopped{};
    }
    co_await async_delay_v2(context->strand(), *delay);
  }
}

template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, recovering &recovering,
                    to_connected_or_stopped_state &event) -> async_t<state_t> {
  spdlog::trace(":::Transition::: recovering -> connected / stopped");

  // TODO: reconnect with policy
  // TODO: add other status check
  // TODO: reset reconnect policy
  if (event.close_state == websocket_close_status::policy_violation) {
    spdlog::trace("stop recovery: close status: {0}", (int)event.close_state);
  }

  try {
    co_await async_reconnect(context);
    co_return connected{};
  } catch (...) {
    co_return stopped{};
  }
}

#pragma endregion

#pragma region STOPPING
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, stopping &stopping,
                    to_stopped_state &event) -> async_t<state_t> {
  spdlog::trace(":::Transition::: stopping -> stopped");
  // TODO: cancel receive loop
  co_await context->receive().async_cancel_message_loop_coro();
  // TODO: cancel sequence loop
  co_return stopped{};
}

#pragma region UNSUPPORTED
// TODO: careful about un-wired transitions
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, auto &state,
                    auto &event) -> async_t<state_t> {
  throw std::logic_error{std::format(":::Transition::: Unsupported state and "
                                     "transition: state: {}, transition: {}",
                                     typeid(state).name(),
                                     typeid(event).name())};
}

template <transition_context_c transition_context_t>
auto async_on_enter(transition_context_t *context, auto &state,
                    auto &event) -> async_t<state_t> {
  spdlog::trace(
      ":::Transition::: Unsupported init state on event: {0}, event: {1}",
      typeid(state).name(), typeid(event).name());
}
#pragma endregion

} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_TRANSITIONS_HPP
