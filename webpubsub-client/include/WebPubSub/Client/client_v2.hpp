#pragma once

#include "webpubsub/client/concepts/websocket_factory_c.hpp"
#include "webpubsub/client/detail/async/exclusion_lock.hpp"
#include "webpubsub/client/detail/logging/log.hpp"
#include "webpubsub/client/detail/services/client_channel_service.hpp"
#include "webpubsub/client/detail/services/client_lifetime_service.hpp"
#include "webpubsub/client/detail/services/client_receive_service.hpp"
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
      : log_(logger_name), channel_service_(strand, log_),
        lifetime_(strand, websocket_factory, channel_service_, log_),
        receive_service_(strand, channel_service_, log_) {
    receive_service_.set_lifetime_service(&lifetime_);
    lifetime_.set_receive_service(&receive_service_);
  }

  auto async_start() -> async_t<> {
    co_await lifetime_.async_raise_event(detail::to_connecting_state{});
    co_await lifetime_.async_raise_event(detail::to_connected_state{});
  }

  auto async_stop() -> async_t<> {
    spdlog::trace("async_stop -- beg");
    co_await lifetime_.async_raise_event(detail::to_stopping_state{});
    co_await lifetime_.async_raise_event(detail::to_stopped_state{});
    spdlog::trace("async_stop -- end");
  }

private:
  detail::client_lifetime_service<websocket_factory_t, websocket_t> lifetime_;
  detail::client_receive_service<websocket_factory_t, websocket_t>
      receive_service_;
  detail::client_channel_service channel_service_;

  const detail::log log_;
  // TODO: DEBUG
  std::string uri_ = "TODO: debug";
};
} // namespace webpubsub