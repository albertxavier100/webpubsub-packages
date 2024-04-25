//
// Created by alber on 2024/3/13.
//

#ifndef TEST_WEBPUBSUB_CLIENT_TRANSITION_CONTEXT_HPP
#define TEST_WEBPUBSUB_CLIENT_TRANSITION_CONTEXT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/client/ack_cache.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/common/utils.hpp"
#include "webpubsub/client/detail/concepts/client_lifetime_service_c.hpp"
#include "webpubsub/client/detail/concepts/client_receive_service_c.hpp"
#include "webpubsub/client/detail/concepts/transition_context_c.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_events.hpp"
#include "webpubsub/client/detail/services/models/transitions.hpp"
#include "webpubsub/client/models/callback_contexts.hpp"

namespace webpubsub {
namespace detail {

/*

stateDiagram-v2
  [*] --> stopped
  stopped --> connecting: to_connecting_state
  connecting --> connected: to_connected
  state should_recover <<choice>>
  connected --> should_recover: should recover?

  should_recover --> recovering: YES to_recovering
  should_recover --> disconnected: NO to_disconnected

  state is_recovered <<choice>>
  recovering --> is_recovered: to_connected_or_disconnected
  is_recovered --> connected: YES
  is_recovered --> disconnected: NO

  disconnected --> reconnecting: to_reconnecting
  state is_reconnected <<choice>>
  reconnecting --> is_reconnected: to_connected_or_stopped
  is_reconnected --> connected: YES
  is_reconnected --> stopped: NO

  connected --> stopping: to_stopping
  disconnected --> stopping: to_stopping

  stopping --> stopped: to_stopped

 * */

// TODO: add concept for receive_t
template <client_lifetime_service_c lifetime_t, typename receive_t,
          typename send_t>
class transition_context {
public:
  transition_context(strand_t &strand, lifetime_t &lifetime, receive_t &receive,
                     send_t &send, const log &log)
      : strand_(strand), state_(stopped{}), lifetime_(lifetime),
        receive_(receive), send_(send), log_(log), ack_id_(0) {
    static_assert(transition_context_c<
                  transition_context<lifetime_t, receive_t, send_t>>);
  }

  eventpp::CallbackList<void(const connected_context)> on_connected;
  eventpp::CallbackList<void(const disconnected_context)> on_disconnected;
  eventpp::CallbackList<void(const group_data_context)> on_group_data;
  eventpp::CallbackList<void(const server_data_context)> on_server_data;
  eventpp::CallbackList<void(const rejoin_group_failed_context)>
      on_rejoin_group_failed;
  eventpp::CallbackList<void(const stopped_context)> on_stopped;

  template <typename callback_context_t>
  auto safe_invoke_callback(callback_context_t && callback_context) {
    // TODO: add log
    try {
      if constexpr (std::is_same_v<connected_context, callback_context_t>) {
        on_connected(callback_context);
        return;
      } else if constexpr (std::is_same_v<disconnected_context,
                                          callback_context_t>) {
        on_disconnected(callback_context);
        return;
      } else if constexpr (std::is_same_v<group_data_context,
                                          callback_context_t>) {
        on_group_data(callback_context);
        return;
      } else if constexpr (std::is_same_v<server_data_context,
                                          callback_context_t>) {
        on_server_data(callback_context);
        return;
      } else if constexpr (std::is_same_v<rejoin_group_failed_context,
                                          callback_context_t>) {
        on_rejoin_group_failed(callback_context);
        return;
      } else if constexpr (std::is_same_v<stopped_context,
                                          callback_context_t>) {
        on_stopped(callback_context);
        return;
      } else {
        static_assert(false, "unsupported context");
        spdlog::trace("unsupported context");
      }
    } catch (const std::exception &ex) {
      // TODO: print callback name
      spdlog::trace("failed to invoke callback: {0}");
    }
  }

  // TODO: remove
  auto test() {}

  auto strand() -> strand_t & { return strand_; }

  auto lifetime() -> lifetime_t & { return lifetime_; }

  auto receive() -> receive_t & { return receive_; }

  auto send() -> send_t & { return send_; }

  auto ack_cache() -> ack_cache & { return ack_cache_; }

  auto next_ack_id() -> uint64_t { return ack_id_++; }

  // TODO: rename
  auto get_state() -> state_t & { return state_; }

  // TODO: [HIGH] ack cache reset
  auto reset() {}

  // TODO: limit this function, only allow use in client_v2
  auto async_raise_event(event_t event) -> async_t<> {
    // TODO: improve it
    state_ = co_await std::visit(overloaded{[this](auto &e) {
                                   return std::visit(
                                       overloaded{[this, &e](auto &s) {
                                         // TODO: better naming
                                         return async_on_event(this, s, e);
                                       }},
                                       state_);
                                 }},
                                 event);

    co_await std::visit(overloaded{[this](auto &e) {
                          return std::visit(overloaded{[this, &e](auto &s) {
                                              return async_on_enter(this, s, e);
                                            }},
                                            state_);
                        }},
                        event);
  }

private:
  // TODO: try to make const
  lifetime_t &lifetime_;
  receive_t &receive_;
  send_t &send_;
  const log &log_;
  state_t state_;
  strand_t &strand_;
  uint64_t ack_id_;
  detail::ack_cache ack_cache_;
};

} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_TRANSITION_CONTEXT_HPP
