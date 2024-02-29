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
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_lifetime_service {
  using strand_t = io::strand<io::io_context::executor_type>;
  // TODO: use optional
  using channel_t = io::experimental::channel<void(io::error_code, bool)>;
  using client_receive_service_t =
      detail::client_receive_service<websocket_factory_t, websocket_t>;
  using client_channel_service_t = detail::client_channel_service;

public:
  client_lifetime_service(strand_t &strand,
                          const websocket_factory_t &websocket_factory,
                          const client_channel_service_t &channel_service,
                          const log &log)
      : log_(log), state_(stopped{}), strand_(strand),
        channel_service_(channel_service),
        websocket_factory_(websocket_factory) {}

  // TODO: IMPL ???
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

  auto async_connect_websocket() -> async_t<> { co_return; }

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
  // TODO: remove
  const client_channel_service_t &channel_service_;
  client_receive_service_t *receive_service_;
  const websocket_factory_t &websocket_factory_;
};

} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_SERVICE_HPP
