//
// Created by alber on 2024/3/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_STOPPING_HPP
#define TEST_WEBPUBSUB_CLIENT_STOPPING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace webpubsub {
namespace detail {
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, stopping &stopping,
                    to_stopped_state &event) -> async_t<state_t> {
  spdlog::trace(":::Transition::: stopping -> stopped");
  co_await context->receive().async_cancel_message_loop_coro();
  spdlog::trace("stopped message loop");
  co_await context->send().async_cancel_sequence_id_loop_coro();
  // TODO: stop connection here.
  // TODO: lock connection
  spdlog::trace("stopped sid loop");
  co_return stopped{};
}
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_STOPPING_HPP
