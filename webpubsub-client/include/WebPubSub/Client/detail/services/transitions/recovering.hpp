//
// Created by alber on 2024/3/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_RECOVERING_HPP
#define TEST_WEBPUBSUB_CLIENT_RECOVERING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "uri.hh"
#include <numeric>

namespace webpubsub {
namespace detail {

// TODO: need unit test
template <transition_context_c transition_context_t>
auto async_recover_connection(transition_context_t *context,
                              to_connected_or_disconnected_state &event)
    -> async_t<state_t> {

  using namespace std::chrono_literals;

  io::steady_timer timeout_timer{context->strand(), 30s};
  auto expiry = timeout_timer.expiry();
  for (;;) {
    try {
      if (expiry < std::chrono::steady_clock::now()) {
        spdlog::trace("Recovery attempts failed more than 30 seconds or the "
                      "client is disconnected");
        spdlog::trace(":::Transition:::  -> disconnected");
        co_return disconnected{};
      }
      auto &lt = context->lifetime();
      co_await lt.async_establish_new_websocket(event.reconnect_uri, context);
      spdlog::trace(":::Transition:::  -> connected");
      co_return connected{};
    } catch (const std::exception &ex) {
      spdlog::trace("fail to recover connection. ex: {0}", ex.what());
    }
    co_await async_delay_v2(context->strand(), 1s);
  }
}

template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, recovering &recovering,
                    to_connected_or_disconnected_state &event)
    -> async_t<state_t> {
  spdlog::trace(":::Transition::: recovering -> connected / disconnected");
  auto next_state = co_await async_recover_connection(context, event);
  co_return next_state;
}
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_RECOVERING_HPP
