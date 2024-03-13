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
                          const log &log)
      : log_(log), strand_(strand), websocket_factory_(websocket_factory) {}

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
        // TODO: IMPL
        //        if (auto delay =) {
        //          co_await async_delay_v2();
        //        }
      }
    }

    // TODO: check if anything else should do before ws connect
    co_await async_connect_websocket();
    co_return;
  }

  auto test() {}

private:
  const log &log_;
  strand_t &strand_;
  const websocket_factory_t &websocket_factory_;
  // const retry_policy reconnect_retry_delay_;
  //  TODO: add connection lock?
};

} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_SERVICE_HPP
