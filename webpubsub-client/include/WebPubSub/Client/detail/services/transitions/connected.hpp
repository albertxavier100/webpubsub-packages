//
// Created by alber on 2024/3/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_FROM_CONNECTED_HPP
#define TEST_WEBPUBSUB_CLIENT_FROM_CONNECTED_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace webpubsub {
namespace detail {

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

// enter connected state
template <transition_context_c transition_context_t>
auto async_on_enter(transition_context_t *context, connected &connected,
                    to_connected_state &event) -> async_t<> {
  spdlog::trace(":::Transition::: enter connected state");
  try {
    context->on_connected(
        connected_context{.connection_id = event.connection_id,
                          .user_id = event.user_id,
                          .reconnection_token = event.reconnection_token});
  } catch (const std::exception &ex) {
  }
  co_return;
}
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_FROM_CONNECTED_HPP
