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
#include "webpubsub/client/detail/concepts/retry_policy_c.hpp"
#include "webpubsub/client/detail/services/client_receive_service.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_events.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_states.hpp"
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
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_lifetime_service {
  using strand_t = io::strand<io::io_context::executor_type>;

public:
  client_lifetime_service(strand_t &strand,
                          const websocket_factory_t &websocket_factory,
                          const bool auto_reconnect,
                          const retry_options &retry_options, const log &log)
      : log_(log), strand_(strand), websocket_factory_(websocket_factory),
        auto_reconnect_(auto_reconnect), retry_policy_(disable_retry_policy()) {
    if (!auto_reconnect_) {
      return;
    }
    switch (retry_options.retry_mode) {
    case retry_mode::exponential:
      retry_policy_.emplace<exponential_retry_policy>(
          exponential_retry_policy(retry_options.max_retry, retry_options.delay,
                                   retry_options.max_delay));
      return;
    case retry_mode::fixed:
      retry_policy_.emplace<fixed_retry_policy>(
          fixed_retry_policy(retry_options.max_retry, retry_options.delay));
      return;
    }
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
        if (auto delay = std::visit(
                overloaded{[](auto &p) { return p.next_retry_delay(); }},
                retry_policy_)) {
          co_await async_delay_v2(*delay);
        } else {
          co_return;
        }
      }
    }
  }

  // TODO: dev
  auto test() {}

  auto auto_reconnect() -> const bool & { return auto_reconnect_; }

private:
  const log &log_;
  strand_t &strand_;
  const websocket_factory_t &websocket_factory_;
  const bool &auto_reconnect_;
  std::variant<fixed_retry_policy, exponential_retry_policy,
               disable_retry_policy>
      retry_policy_;
  //  TODO: add connection lock?
};

} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_SERVICE_HPP
