//
// Created by alber on 2024/3/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_FROM_STOPPED_HPP
#define TEST_WEBPUBSUB_CLIENT_FROM_STOPPED_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace webpubsub {
namespace detail {
// stopped -> connecting
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, stopped &stopped,
                    to_connecting_state &event) -> async_t<state_t> {
  spdlog::trace(":::Transition::: stopped -> connecting: reset connection");
  co_return connecting{};
}
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_FROM_STOPPED_HPP
