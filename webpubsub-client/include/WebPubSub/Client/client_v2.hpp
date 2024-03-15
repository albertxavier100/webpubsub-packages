#pragma once

#include "eventpp/callbacklist.h"
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
      : log_(logger_name),
        lifetime_(strand, websocket_factory, options.auto_reconnect,
                  options.reconnect_retry_options, log_),
        receive_(strand, log_), transition_context_(lifetime_, receive_, log_),
        on_connected(transition_context_.on_connected),
        on_disconnected(transition_context_.on_disconnected),
        on_group_data(transition_context_.on_group_data),
        on_server_data(transition_context_.on_server_data),
        on_rejoin_group_failed(transition_context_.on_rejoin_group_failed),
        on_stopped(transition_context_.on_stopped) {}

  eventpp::CallbackList<void(const connected_context)> &on_connected;
  eventpp::CallbackList<void(const disconnected_context)> &on_disconnected;
  eventpp::CallbackList<void(const group_data_context)> &on_group_data;
  eventpp::CallbackList<void(const server_data_context)> &on_server_data;
  eventpp::CallbackList<void(const rejoin_group_failed_context)>
      &on_rejoin_group_failed;
  eventpp::CallbackList<void(const stopped_context)> &on_stopped;

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
  detail::client_lifetime_service<websocket_factory_t, websocket_t> lifetime_;
  detail::client_receive_service receive_;

  detail::transition_context<
      detail::client_lifetime_service<websocket_factory_t, websocket_t>,
      detail::client_receive_service>
      transition_context_;
  const detail::log log_;
  io::cancellation_signal dummy_start_signal_;
};
} // namespace webpubsub