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

struct to_connecting_state {};
struct to_connected_state {};
struct to_stopped_state {};

using event_t =
    std::variant<to_stopped_state, to_connected_state, to_connecting_state>;
template <typename t = void> using async_t = io::awaitable<t>;

struct connected {};

struct connecting {
  connecting() = default;

  template <typename t> auto async_move_to(event_t ev) -> async_t<t> {
    spdlog::trace("connecting -> connected: establish websocket connection");
    co_return connected{};
  }
};

struct stopped {

  stopped() = default;

  template <typename t> auto async_move_to(event_t ev) -> async_t<t> {
    spdlog::trace("stopped -> connecting: reset connection");
    co_return connecting{};
  }
};

struct recovering {};
struct stopping {};

using state_t =
    std::variant<stopped, connecting, connected, recovering, stopping>;

} // namespace lifetime
} // namespace detail
} // namespace webpubsub

// TODO: move to new file
namespace webpubsub {
namespace detail {
namespace lifetime {
// TODO: move to new file
struct state_visitor {

  using async_move_to_t = auto(event_t) -> async_t<state_t>;

  // TODO: IMPL
  void operator()(stopped &s) {
    async_move_to =
        std::bind(&stopped::async_move_to<state_t>, &s, std::placeholders::_1);
  };
  void operator()(connecting &s) {
    async_move_to = std::bind(&connecting::async_move_to<state_t>, &s,
                              std::placeholders::_1);
  };
  // TODO: IMPL
  void operator()(connected &s) {};
  // TODO: IMPL
  void operator()(recovering &s) {};
  // TODO: IMPL
  void operator()(stopping &s) {};
  std::function<async_move_to_t> async_move_to;
};

template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_lifetime {
  using strand_t = io::strand<io::io_context::executor_type>;
  // TODO: use optional
  using channel_t = io::experimental::channel<void(io::error_code, bool)>;

public:
  client_lifetime(strand_t &strand,
                  const websocket_factory_t &websocket_factory, const log &log)
      : log_{log}, state_{stopped{}}, strand_{strand}, channel_{strand, 1},
        websocket_factory_{websocket_factory} {}

  // TODO: IMPL
  auto async_handle_event(event_t ev) -> async_t<> {
    spdlog::trace("lifetime.async_handle_event begin");
    state_visitor sta_visitor;
    std::visit(sta_visitor, state_);
    spdlog::trace("std::visit(sta_visitor begin");
    state_ = co_await sta_visitor.async_move_to(std::move(ev));
    co_return;
  }

  auto get_state() { return state_; }

  template <typename t> auto is_in_state() {
    return std::holds_alternative<t>(state_);
  }

private:
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
