#pragma once

#include "webpubsub/client/concepts/websocket_factory_c.hpp"
#include "webpubsub/client/detail/async/exclusion_lock.hpp"
#include "webpubsub/client/detail/logging/log.hpp"
#include "webpubsub/client/detail/services/client_channel_service.hpp"
#include "webpubsub/client/detail/services/client_lifetime_service.hpp"
#include "webpubsub/client/detail/services/client_receive_service.hpp"
#include "webpubsub/client/detail/services/models/transition_context.hpp"
#include "webpubsub/client/exceptions/exception.hpp"
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
      : log_(logger_name), lifetime_(strand, websocket_factory, log_),
        receive_(strand, log_), transition_context_(lifetime_, receive_, log_) {
  }

  // TODO: make start_slot optional
  auto async_start(std::optional<io::cancellation_slot> slot = std::nullopt)
      -> async_t<> {
    co_await transition_context_.async_raise_event(
        detail::to_connecting_state{});
    auto event = detail::to_connected_state{};
    if (slot) {
      event.start_slot = *slot;
    } else {
      event.start_slot = dummy_start_signal_.slot();
    }
    co_await transition_context_.async_raise_event(std::move(event));
  }

  auto async_stop() -> async_t<> {
    spdlog::trace("async_stop -- beg");
    co_await transition_context_.async_raise_event(detail::to_stopping_state{});
    co_await transition_context_.async_raise_event(detail::to_stopped_state{});
    spdlog::trace("async_stop -- end");
  }

private:
  // TODO: remove
  detail::client_lifetime_service<websocket_factory_t, websocket_t> lifetime_;
  // TODO: remove
  detail::client_receive_service receive_;

  detail::transition_context<
      detail::client_lifetime_service<websocket_factory_t, websocket_t>,
      detail::client_receive_service>
      transition_context_;
  const detail::log log_;
  io::cancellation_signal dummy_start_signal_;
};
} // namespace webpubsub