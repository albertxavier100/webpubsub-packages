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
#include "webpubsub/client/models/client_options.hpp"
#include <optional>

namespace webpubsub {
namespace detail {

// TODO: move to new file
template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// TODO: rename websocket_service
template <webpubsub_protocol_t protocol_t, typename websocket_factory_t,
          typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>

class client_lifetime_service {
  using strand_t = io::strand<io::io_context::executor_type>;
  using retry_policy_t =
      std::variant<fixed_retry_policy, exponential_retry_policy,
                   disable_retry_policy>;

public:
  client_lifetime_service(strand_t &strand,
                          websocket_factory_t &websocket_factory,
                          const client_options<protocol_t> &options,
                          const log &log)
      : log_(log), strand_(strand), options_(options),
        websocket_factory_(websocket_factory), websocket_(nullptr),
        retry_policy_(disable_retry_policy()) {
    if (!options.auto_reconnect) {
      return;
    }
    const auto &max_retry = options.reconnect_retry_options.max_retry;
    const auto &max_delay = options.reconnect_retry_options.max_delay;
    const auto &delay = options.reconnect_retry_options.delay;
    switch (options.reconnect_retry_options.retry_mode) {
    case retry_mode::exponential: {
      retry_policy_.emplace<exponential_retry_policy>(
          exponential_retry_policy(max_retry, delay, max_delay));
      return;
    }
    case retry_mode::fixed: {
      retry_policy_.emplace<fixed_retry_policy>(
          fixed_retry_policy(max_retry, delay));
      return;
    }
    }
  }

  // TODO: IMPL
  auto async_connect_new_websocket() -> async_t<> {
    spdlog::trace("async_connect_websocket -- begin");
    reset_websocket_connection();
    // TODO: actually connect
    co_return;
  }

  /*
//  auto async_auto_reconnect() -> async_t<> {
//    bool ok = false;
//
//    struct exit_scope {
//      ~exit_scope() {
//        if (!ok) {
//          // TODO: IMPL: handle client stop
//        }
//      }
//    } _;
//
//    auto retry = 0;
//
//    for (;;) {
//      auto cs = co_await io::this_coro::cancellation_state;
//      if (cs.cancelled() != io::cancellation_type::none) {
//        spdlog::trace("auto reconnect break;");
//        break;
//      }
//      bool should_retry = false;
//      try {
//        co_await async_connect_websocket();
//        ok = true;
//        co_return;
//      } catch (const std::exception &ex) {
//        should_retry = true;
//      }
//      if (should_retry) {
//        spdlog::trace("fail to reconnect");
//        retry++;
//        if (auto delay = std::visit(
//                overloaded{[](auto &p) { return p.next_retry_delay(); }},
//                retry_policy_)) {
//          co_await async_delay_v2(*delay);
//        } else {
//          co_return;
//        }
//      }
//    }
//  }
*/

  auto
  async_read_message(std::string &payload,
                     webpubsub::websocket_close_status &status) -> async_t<> {
    spdlog::trace("lifetime.async_read_message");
    co_await websocket_->async_read(payload, status);
  }

  // TODO: IMPL
  auto reset_websocket_connection() {
    websocket_ = websocket_factory_.create("url", options_.protocol.get_name());
  }

  // TODO: dev
  auto test() {}

  auto auto_reconnect() -> const bool & { return options_.auto_reconnect; }
  auto retry_policy() -> retry_policy_t & { return retry_policy_; };

private:
  const log &log_;
  strand_t &strand_;
  websocket_factory_t &websocket_factory_;
  std::unique_ptr<websocket_t> websocket_;
  const client_options<protocol_t> &options_;

  retry_policy_t retry_policy_;
  //  TODO: add connection lock?
};

} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_SERVICE_HPP
