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
#include "webpubsub/client/models/callback_contexts.hpp"

namespace webpubsub {
namespace detail {

// TODO: add concept for receive_t
template <client_lifetime_service_c lifetime_t, typename receive_t>
class transition_context {
public:
  transition_context(strand_t &strand, lifetime_t &lifetime, receive_t &receive,
                     const log &log)
      : strand_(strand), state_(stopped{}), lifetime_(lifetime),
        receive_(receive), log_(log) {
    static_assert(
        transition_context_c<transition_context<lifetime_t, receive_t>>);

    // TODO: move callback to client_v2.hpp
    receive_.on_receive_failed.append([this](const bool _) {
      io::co_spawn(
          strand_,
          [this]() -> async_t<> {
            try {

              spdlog::trace("on_receive_failed.recovering... beg");
              spdlog::trace("on_receive_failed.recovering... to disconnected");
              co_await async_raise_event(to_disconnected_state{
                  .connection_id = "TODO", .reason = "TODO"}); // TODO
              spdlog::trace(
                  "on_receive_failed.recovering... to recovering or stopped");
              co_await async_raise_event(to_recovering_or_stopped_state{});
              spdlog::trace("on_receive_failed.recovering... to "
                            "to_connected_or_stopped_state");
              co_await async_raise_event(to_connected_or_stopped_state{});
              spdlog::trace("on_receive_failed.recovering... end");
            } catch (const std::exception &ex) {
              spdlog::trace("failed to recover, ex: {0}", ex.what());
              throw;
            }
          },
          io::detached);
    });
  }

  eventpp::CallbackList<void(const connected_context)> on_connected;
  eventpp::CallbackList<void(const disconnected_context)> on_disconnected;
  eventpp::CallbackList<void(const group_data_context)> on_group_data;
  eventpp::CallbackList<void(const server_data_context)> on_server_data;
  eventpp::CallbackList<void(const rejoin_group_failed_context)>
      on_rejoin_group_failed;
  eventpp::CallbackList<void(const stopped_context)> on_stopped;

  // TODO: remove
  auto test() {}

  auto strand() -> strand_t & { return strand_; }

  auto lifetime() -> lifetime_t & { return lifetime_; }

  auto receive() -> receive_t & { return receive_; }

  auto get_state() -> const state_t & { return state_; }

  auto async_raise_event(event_t event) -> async_t<> {
    state_ = co_await std::visit(
        overloaded{[this](auto &e) {
          return std::visit(
              overloaded{[this, &e](auto &s) {
                return [this, &e, &s]() -> async_t<state_t> {
                  // TODO: better naming
                  auto state = co_await async_on_event(this, s, e);
                  co_await std::visit(overloaded{[this, &e](auto &next_state) {
                                        return async_on_enter(this, next_state,
                                                              e);
                                      }},
                                      state);
                  co_return state;
                }();
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
  strand_t &strand_;
};

} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_TRANSITION_CONTEXT_HPP
