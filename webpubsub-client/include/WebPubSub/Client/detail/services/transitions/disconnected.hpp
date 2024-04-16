//
// Created by alber on 2024/3/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_DISCONNECTED_HPP
#define TEST_WEBPUBSUB_CLIENT_DISCONNECTED_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace webpubsub {
namespace detail {
// disconnected => recovering or stopped
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, disconnected &disconnected,
                    to_reconnecting_state &event) -> async_t<state_t> {
  spdlog::trace(":::Transition::: disconnected -> reconnecting");

  // TODO: finish any awaiting ack entity
  if (context->lifetime().auto_reconnect()) {
    spdlog::trace(":::Transition::: -> reconnecting");
    auto retry_context = context->lifetime.make_retry_context();
    co_return reconnecting{retry_context};
  }
  try {
    context->on_stopped(stopped_context{});
  } catch (const std::exception &ex) {
    spdlog::trace("failed to invoke on stopped event: {0}", ex.what());
  }
  spdlog::trace(":::Transition::: -> stopped");
  co_return stopped{};
}

// enter disconnected state
template <transition_context_c transition_context_t>
auto async_on_enter(transition_context_t *context, disconnected &disconnected,
                    to_disconnected_state &event) -> async_t<> {
  spdlog::trace(":::Transition::: enter disconnected state");
  try {
    context->on_disconnected(
        disconnected_context{std::move(event.connection_id),
                             std::move(event.reason)}); // TODO: get from event
  } catch (const std::exception &ex) {
    spdlog::trace("failed to invoke on_disconnected");
  }
  co_return;
}
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_DISCONNECTED_HPP
