//
// Created by alber on 2024/3/13.
//

#ifndef TEST_WEBPUBSUB_CLIENT_TRANSITION_CONTEXT_C_HPP
#define TEST_WEBPUBSUB_CLIENT_TRANSITION_CONTEXT_C_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_events.hpp"
#include "webpubsub/client/models/callback_contexts.hpp"

namespace webpubsub {
namespace detail {
template <typename transition_context_t>
concept transition_context_c = requires(transition_context_t t) {
  { t.async_raise_event(event_t()) } -> std::same_as<async_t<>>;
  //  {
  //    t.on_connected
  //  } -> std::same_as<eventpp::CallbackList<void(const connected_context)>>;
  //  {
  //    t.on_disconnected
  //  } -> std::same_as<eventpp::CallbackList<void(const
  //  disconnected_context)>>;
  //  {
  //    t.on_group_data
  //  } -> std::same_as<eventpp::CallbackList<void(const group_data_context)>>;
  //  {
  //    t.on_server_data
  //  } -> std::same_as<eventpp::CallbackList<void(const server_data_context)>>;
  //  {
  //    t.on_rejoin_group_failed
  //  } -> std::same_as<
  //        eventpp::CallbackList<void(const rejoin_group_failed_context)>>;
  //  {
  //    t.on_stopped
  //  } -> std::same_as<eventpp::CallbackList<void(const stopped_context)>>;
  //    { t.lifetime() } -> std::same_as<>;
  // TODO: add constraint
  //  { t.spawn_message_loop_coro() } -> std::same_as<void>;
  { t.test() } -> std::same_as<void>;
};
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_TRANSITION_CONTEXT_C_HPP
