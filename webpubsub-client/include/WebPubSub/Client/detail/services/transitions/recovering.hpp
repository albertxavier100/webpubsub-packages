//
// Created by alber on 2024/3/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_RECOVERING_HPP
#define TEST_WEBPUBSUB_CLIENT_RECOVERING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace webpubsub {
namespace detail {

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
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_RECOVERING_HPP
