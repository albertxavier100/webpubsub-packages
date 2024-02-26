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
#include "webpubsub/client/detail/async/utils.hpp"
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
                  const websocket_factory_t &websocket_factory, const log &log)
      : log_{log}, state_{state::stopped{}}, strand_{strand},
        channel_{strand, 1}, websocket_factory_{websocket_factory} {}

  auto async_start(event_t e) -> async_t<> {
    spdlog::trace("lifetime.async_start begin");
    handle_event(e, [](std::exception_ptr ex) {

    });
    co_return;
  }

private:
  // TODO: IMPL
  auto handle_event(event_t ev,
                    std::function<void(std::exception_ptr ex)> callback) {
    using namespace state;
    using namespace event;

    spdlog::trace("lifetime.async_handle_event begin");

    // TODO: return wps result
    match(
        ev, //
        [this, &callback](const event::start &e) {
          io::co_spawn(strand_, async_start_core(std::move(e)), callback);
        },
        [this, &callback](const stop &e) {
          // TODO: remove
          io::co_spawn(strand_, async_stop_core(std::move(e)), callback);
        });
  }

  // TODO: IMPL
  auto async_stop_core(event::stop e) -> async_t<> { co_return; }

  // TODO: IMPL
  auto async_start_core(event::start e) -> async_t<> {
    using namespace state;
    using namespace event;
    spdlog::trace("lifetime.async_start_core begin");
    match(state_, //
          [this, &e](auto &s) {
    spdlog::trace("lifetime.async_start_core.match begin");
            using t = std::decay_t<decltype(s)>;
            if constexpr (!std::is_same_v<t, stopped>) {
              spdlog::trace("lifetime.async_start_core.match.throw begin");
              throw invalid_operation{
                  "TODO fail to connect in non stopped state"};
            }
            state_ = connecting{};
            io::co_spawn(strand_, async_connect(e), [this](auto ex) {
              state_ = connected{};
              spdlog::trace("client connected");
            });
          });
    co_return;
  }

  // TODO: IMPL
  auto async_connect(event::start e) -> async_t<> { co_return; }

  // TODO: IMPL
  auto async_stop_receiving_service() -> async_t<> { co_return; }

  // TODO: IMPL
  auto async_stop_sending_service() -> async_t<> { co_return; }

  const log &log_;

  state_t state_;

  strand_t &strand_;
  const channel_t channel_;

  const websocket_factory_t &websocket_factory_;
};
} // namespace lifetime
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_HPP
