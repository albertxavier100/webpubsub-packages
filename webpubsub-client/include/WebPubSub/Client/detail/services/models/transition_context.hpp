//
// Created by alber on 2024/3/13.
//

#ifndef TEST_WEBPUBSUB_CLIENT_TRANSITION_CONTEXT_HPP
#define TEST_WEBPUBSUB_CLIENT_TRANSITION_CONTEXT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/concepts/client_lifetime_service_c.hpp"
#include "webpubsub/client/detail/concepts/client_receive_service_c.hpp"
#include "webpubsub/client/detail/concepts/transition_context_c.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_events.hpp"
#include "webpubsub/client/detail/services/models/transitions.hpp"

namespace webpubsub {
namespace detail {

// TODO: add concept for receive_t
template <client_lifetime_service_c lifetime_t, typename receive_t>
class transition_context {
public:
  transition_context(lifetime_t &lifetime, receive_t &receive, const log &log)
      : state_(stopped{}), lifetime_(lifetime), receive_(receive), log_(log) {

    static_assert(
        transition_context_c<transition_context<lifetime_t, receive_t>>);
  }

  // TODO: remove
  auto test() {}

  //  auto async_cancel_message_loop_coro() -> async_t<> {
  //    co_await receive_.async_cancel_message_loop_coro();
  //  }

  //  auto spawn_message_loop_coro(const transition_context &context,
  //                               io::cancellation_slot start_slot) {
  //    receive_.spawn_message_loop_coro(context, std::move(start_slot));
  //  }

  auto &lifetime() { return lifetime_; }

  auto &receive() { return receive_; }

  auto get_state() { return state_; }

  auto async_raise_event(event_t event) -> async_t<> {
    state_ = co_await std::visit(overloaded{[this](auto &e) {
                                   return std::visit(
                                       overloaded{[this, &e](auto &s) {
                                         return async_on_event(this, s, e);
                                       }},
                                       state_);
                                 }},
                                 event);
  }

private:
  // TODO: try to make const
  lifetime_t &lifetime_;
  receive_t &receive_;
  const log &log_;
  state_t state_;
};

} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_TRANSITION_CONTEXT_HPP
