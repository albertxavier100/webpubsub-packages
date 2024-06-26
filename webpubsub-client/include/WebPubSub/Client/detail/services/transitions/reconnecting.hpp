//
// Created by alber on 2024/3/27.
//

#ifndef TEST_WEBPUBSUB_CLIENT_RECONNECTING_HPP
#define TEST_WEBPUBSUB_CLIENT_RECONNECTING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace webpubsub {
namespace detail {
template <transition_context_c transition_context_t>
auto async_reconnect(transition_context_t *context, reconnecting &reconnecting,
                     to_connected_or_stopped_state &event) -> async_t<state_t> {
  using namespace std::chrono_literals;
  for (;;) {
    try {
      co_await context->lifetime().async_connect(context);
      spdlog::trace("reconnect successfully.");
      spdlog::trace(":::Transition::: -> connected");
      co_return connected{};
    } catch (const std::exception &ex) {
      spdlog::trace("failed to reconnect. {0}", ex.what());
    }
    auto delay = reconnecting.retry_context.delay;
    if (!delay) {
      spdlog::trace(":::Transition::: -> stopped");
      co_return stopped{};
    }
    co_await async_delay_v2(context->strand(), *delay);
  }
}

template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, reconnecting &reconnecting,
                    to_connected_or_stopped_state &event) -> async_t<state_t> {
  spdlog::trace(":::Transition::: reconnecting -> connected / stopped");

  auto next_state = co_await async_reconnect(context, reconnecting, event);
  co_return next_state;
}

template <transition_context_c transition_context_t>
auto async_on_enter(transition_context_t *context, reconnecting &reconnecting,
                    to_reconnecting_state &event) -> async_t<> {
  update_delay(reconnecting.retry_context);
  co_return;
}
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_RECONNECTING_HPP
