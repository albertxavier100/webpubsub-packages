//
// Created by alber on 2024/3/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_FROM_CONNECTING_HPP
#define TEST_WEBPUBSUB_CLIENT_FROM_CONNECTING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace webpubsub {
namespace detail {
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, connecting &connecting,
                    to_connected_state &event) -> async_t<state_t> {
  spdlog::trace(":::Transition::: connecting -> connected ");
  try {
    co_await context->lifetime().async_connect(context);
    co_return connected{};
  } catch (const std::exception &ex) {
    spdlog::trace("failed to connect to websocket");
    throw invalid_operation("failed to connect to websocket");
  }
}
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_FROM_CONNECTING_HPP
