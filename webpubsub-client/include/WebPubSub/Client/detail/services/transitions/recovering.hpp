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
auto async_reconnect_with_retry(transition_context_t *context)
    -> async_t<state_t> {
  auto retry_policy = context->lifetime().retry_policy();
  for (;;) {
    try {
      co_await context->lifetime().async_connect_new_websocket();
      spdlog::trace("reconnect successfully.");
      co_return connected{};
    } catch (const std::exception &ex) {
      spdlog::trace("failed to reconnect. {0}", ex.what());
    }
    auto delay = std::visit(
        overloaded{[](auto &policy) { return policy.next_retry_delay(); }},
        retry_policy);
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

  // TODO: add other status check
  if (event.close_state == websocket_close_status::policy_violation) {
    spdlog::trace("stop recovery: close status: {0}", (int)event.close_state);
  }
  auto next_state = co_await async_reconnect_with_retry(context);
  co_return next_state;
}
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_RECOVERING_HPP
