//
// Created by alber on 2024/2/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/concepts/websocket_factory_c.hpp"
#include "webpubsub/client/detail/common/match.hpp"
#include "webpubsub/client/exceptions/exception.hpp"
#include <optional>

namespace webpubsub {
namespace detail {
namespace lifetime {
namespace state {
struct stopped {};
struct connecting {};
struct connected {};
struct recovering {};
struct stopping {};
} // namespace state
} // namespace lifetime
} // namespace detail
} // namespace webpubsub

namespace webpubsub {
namespace detail {
namespace lifetime {
namespace event {
struct start {
  std::string uri;
};
struct stop {};
struct join_group {};

} // namespace event
} // namespace lifetime
} // namespace detail
} // namespace webpubsub

namespace webpubsub {
namespace detail {
namespace lifetime {
template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_lifetime {
  using strand_t = io::strand<io::io_context::executor_type>;
  // TODO: use optional
  using state_t =
      std::variant<state::stopped, state::connecting, state::connected,
                   state::recovering, state::stopping>;
  using event_t = std::variant<event::start>;
  using channel_t = io::experimental::channel<void(io::error_code, bool)>;
  template <typename T = void> using async_t = io::awaitable<T>;

public:
  client_lifetime(strand_t &strand,
                  const websocket_factory_t &websocket_factory, log &log)
      : log_{log}, state_{state::stopped{}}, strand_{strand},
        channel_{strand, 1}, websocket_factory_{websocket_factory} {}

  auto async_start(event_t e) -> async_t<> {
    spdlog::trace("lifetime.async_start begin");
    co_await async_handle_event(e);
  }
  auto async_stop(event_t e) -> async_t<> { co_await async_handle_event(e); }

private:
  // TODO: IMPL
  auto async_handle_event(event_t ev) -> async_t<> {
    using namespace state;
    using namespace event;

    spdlog::trace("lifetime.async_handle_event begin");
    co_await match(
        std::move(ev), //
        [this](start e) -> async_t<> {
          co_await async_start_core(std::move(e));
        },
        [this](stop e) -> async_t<> {
          co_await async_stop_core(std::move(e));
        });
  }

  // TODO: IMPL
  auto async_stop_core(event::stop e) -> async_t<> {
    using namespace io::experimental::awaitable_operators;
    state_ = co_await match(state_, [&](auto &s) -> async_t<state_t> {
      using namespace state;
      using namespace event;

      if constexpr (std::is_same_v<decltype(s), stopped>) {
        co_return s;
      } else {
        co_await (async_stop_receiving_service() &&
                  async_stop_sending_service());
      }
    });
  }

  // TODO: IMPL
  auto async_start_core(event::start e) -> async_t<> {
    state_ = co_await match(state_, [&](auto &s) -> async_t<state_t> {
      using namespace state;
      using namespace event;

      spdlog::trace("lifetime.async_start_core begin");
      if constexpr (std::is_same_v<decltype(s), stopped>) {
        co_await async_connect(std::move(e));
        spdlog::trace("connected");
        co_return connected{};
      } else {
        throw invalid_operation{"Can not start a client during stopping"};
      }
    });
  }

  // TODO: IMPL
  auto async_connect(event::start e) -> async_t<> { co_return; }

  // TODO: IMPL
  auto async_stop_receiving_service() -> async_t<> { co_return; }

  // TODO: IMPL
  auto async_stop_sending_service() -> async_t<> { co_return; }

  const log log_;

  state_t state_;

  strand_t &strand_;
  const channel_t channel_;

  const websocket_factory_t &websocket_factory_;
};
} // namespace lifetime
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_HPP
