#pragma once

#include "webpubsub/client/concepts/websocket_factory_c.hpp"
#include "webpubsub/client/detail/async/exclusion_lock.hpp"
#include "webpubsub/client/detail/logging/log.hpp"
#include "webpubsub/client/detail/services/client_lifetime_service.hpp"
#include "webpubsub/client/detail/services/client_receive_service.hpp"
#include "webpubsub/client/models/client_options.hpp"

namespace webpubsub {
template <webpubsub_protocol_t protocol_t, typename websocket_factory_t,
          typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_v2 {
  template <typename t = void> using async_t = io::awaitable<t>;

public:
  client_v2(io::strand<io::io_context::executor_type> &strand,
            const client_options<protocol_t> &options,
            const websocket_factory_t &websocket_factory,
            const std::string &logger_name)
      : log_(logger_name), lifetime_{strand, websocket_factory, log_} {}

  auto async_start(const io::cancellation_slot slot) -> async_t<> {
    co_await lifetime_.async_handle_event(detail::to_connecting_state{});
    if (!lifetime_.is_in_state<detail::connecting>()) {
      throw invalid_operation(
          "failed to to start connection due to resetting connection failure");
    }
    co_await lifetime_.async_handle_event(detail::to_connected_state{});
    if (!lifetime_.is_in_state<detail::connected>()) {
      throw invalid_operation("failed to start connection due to establishing "
                              "websocket connection failure");
    }
  }

private:
  detail::client_lifetime_service<websocket_factory_t, websocket_t> lifetime_;
  const detail::log log_;

  // TODO: DEBUG
  std::string uri_ = "TODO: debug";
};
} // namespace webpubsub