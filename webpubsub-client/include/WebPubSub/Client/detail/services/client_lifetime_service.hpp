//
// Created by alber on 2024/2/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_SERVICE_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/concepts/websocket_factory_c.hpp"
#include "webpubsub/client/detail/async/utils.hpp"
#include "webpubsub/client/detail/client/retry_policy.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/services/client_receive_service.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_events.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_states.hpp"
#include "webpubsub/client/detail/services/models/transitions.hpp"
#include "webpubsub/client/exceptions/exception.hpp"
#include <optional>

namespace webpubsub {
namespace detail {

// TODO: move to new file
template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <typename websocket_factory_t, typename websocket_t>
class client_lifetime_service {
  using strand_t = io::strand<io::io_context::executor_type>;
  // TODO: use optional
  using channel_t = io::experimental::channel<void(io::error_code, bool)>;
  using client_receive_service_t =
      detail::client_receive_service<websocket_factory_t, websocket_t>;

public:
  client_lifetime_service(strand_t &strand,
                          const websocket_factory_t &websocket_factory,
                          const retry_policy_c &retry_policy, const log &log)
    requires websocket_factory_c<websocket_factory_t, websocket_t> &&
                 retry_policy_c<retry_policy_t>
      : log_(log), state_(stopped{}), strand_(strand),
        websocket_factory_(websocket_factory) {}

  // TODO: IMPL
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

  auto async_connect_websocket() -> async_t<> {
    spdlog::trace("async_connect_websocket -- begin");
    co_return;
  }

  auto async_auto_reconnect() -> async_t<> {
    bool ok = false;

    struct exit_scope {
      ~exit_scope() {
        if (!ok) {
          // TODO: IMPL: handle client stop
        }
      }
    } _;

    auto retry = 0;

    for (;;) {
      auto cs = co_await io::this_coro::cancellation_state;
      if (cs.cancelled() != io::cancellation_type::none) {
        spdlog::trace("auto reconnect break;");
        break;
      }
      bool should_retry = false;
      try {
        co_await async_connect_websocket();
        ok = true;
        co_return;
      } catch (const std::exception &ex) {
        should_retry = true;
      }
      if (should_retry) {
        spdlog::trace("fail to reconnect");
        retry++;
        if (auto delay =) {
          co_await async_delay_v2();
        }
      }
    }

    // TODO: check if anything else should do before ws connect
    co_await async_connect_websocket();
    co_return;
  }

  auto get_state() { return state_; }

  template <typename t> auto is_in_state() {
    return std::holds_alternative<t>(state_);
  }

  auto set_receive_service(client_receive_service_t *receive_service) {
    receive_service_ = receive_service;
  }

  auto get_receive_service() { return receive_service_; }

private:
  const log &log_;
  state_t state_;
  strand_t &strand_;
  client_receive_service_t *receive_service_;
  const websocket_factory_t &websocket_factory_;
  const retry_policy reconnect_retry_delay_;
  // TODO: add connection lock?
};

} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_SERVICE_HPP
