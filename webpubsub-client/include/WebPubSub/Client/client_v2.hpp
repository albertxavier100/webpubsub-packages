#pragma once

#include "webpubsub/client/concepts/websocket_factory_c.hpp"
#include "webpubsub/client/detail/async/exclusion_lock.hpp"
#include "webpubsub/client/detail/logging/log.hpp"
#include "webpubsub/client/detail/services/client_lifetime.hpp"
#include "webpubsub/client/models/client_options.hpp"

namespace webpubsub {
template <webpubsub_protocol_t protocol_t, typename websocket_factory_t,
          typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_v2 {
  template <typename T = void> using async_t = io::awaitable<T>;
  template <typename T, typename U>
  using client_lifetime_t = detail::lifetime::client_lifetime<T, U>;

public:
  client_v2(io::strand<io::io_context::executor_type> &strand,
            const client_options<protocol_t> &options,
            const websocket_factory_t &websocket_factory,
            const std::string &logger_name)
      : log_(logger_name), lifetime_{strand, websocket_factory, log_} {}

  auto async_start(const io::cancellation_slot slot) -> async_t<> {
    using namespace detail::lifetime;
    co_await lifetime_.async_handle_event(
        detail::lifetime::to_connecting_state{});
    if (!lifetime_.is_in_state<connecting>()) {
      throw invalid_operation(
          "failed to to start connection due to resetting connection failure");
    }
    co_await lifetime_.async_handle_event(to_connected_state{});
    if (!lifetime_.is_in_state<connected>()) {
      throw invalid_operation("failed to start connection due to establishing "
                              "websocket connection failure");
    }
  }

private:
  client_lifetime_t<websocket_factory_t, websocket_t> lifetime_;
  const detail::log log_;

  // TODO: DEBUG
  std::string uri_ = "TODO: debug";
};
} // namespace webpubsub