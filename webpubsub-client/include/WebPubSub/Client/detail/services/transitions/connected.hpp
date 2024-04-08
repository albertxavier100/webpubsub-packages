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

template <transition_context_c transition_context_t>
auto async_spawn_loops(transition_context_t context,
                       io::cancellation_slot start_slot) -> async_t<> {
  spdlog::trace("spawn_sequence_ack_loop_coro");
  context->send().spawn_sequence_ack_loop_coro(context, start_slot);
  spdlog::trace("spawn_message_loop_coro");
  // TODO: cannot use the same slot
  context->receive().spawn_message_loop_coro(context, start_slot);
}

// enter connected state
template <transition_context_c transition_context_t>
auto async_on_enter(transition_context_t *context, connected &connected,
                    to_connected_state &event) -> async_t<> {
  spdlog::trace(":::Transition::: enter connected state");
  try {
    // TODO: use real string
    context->on_connected(connected_context{.connection_id = "TODO",
                                            .user_id = "TODO",
                                            .reconnection_token = "TODO"});
    
    spdlog::trace("spawn_sequence_ack_loop_coro");
    context->send().spawn_sequence_ack_loop_coro(context->cancel_signal.slot());
    spdlog::trace("spawn_message_loop_coro");
    context->receive().spawn_message_loop_coro(context);
  } catch (const std::exception &ex) {
    spdlog::trace("get ex in on enter connected state, ex: {0}", ex.what());
  }
  co_return;
}

template <transition_context_c transition_context_t>
auto async_on_enter(transition_context_t *context, connected &connected,
                    to_connected_or_disconnected_state &event) -> async_t<> {
  spdlog::trace(":::Transition::: enter connected state");
  try {
    spdlog::trace("spawn_sequence_ack_loop_coro - ");
    context->send().spawn_sequence_ack_loop_coro(context->cancel_signal.slot());
    spdlog::trace("spawn_message_loop_coro");
    context->receive().spawn_message_loop_coro(context);
  } catch (const std::exception &ex) {
    spdlog::trace("get ex in on enter connected state, ex: {0}", ex.what());
  }
  co_return;
}

} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_FROM_CONNECTED_HPP
