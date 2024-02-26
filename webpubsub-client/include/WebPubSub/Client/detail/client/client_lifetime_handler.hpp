#pragma once

#include "webpubsub/client/deps/client_lifetime_handler.hpp"
#include "webpubsub/client/detail/async/exclusion_lock.hpp"
#include "webpubsub/client/detail/client/client_responser.hpp"
#include "webpubsub/client/detail/client/task_tracker.hpp"
#include "webpubsub/client/detail/logging/log.hpp"
#include "webpubsub/client/models/client_options.hpp"

namespace webpubsub {
namespace detail {
template <webpubsub_protocol_t protocol, typename websocket_factory,
          typename websocket>
  requires websocket_factory_t<websocket_factory, websocket>
class client_lifetime_handler {

public:
  client_lifetime_handler(
      lib::asio::strand<lib::asio::io_context::executor_type> &strand,
      const client_options<protocol> &options,
      const websocket_factory &websocket_factory,
      std::shared_ptr<spdlog::logger> logger)
      : log_(std::move(logger)), strand_(strand),
        responser_(strand, options.protocol.is_reliable(), tracker_, log_) {}
  auto async_start() -> lib::asio::awaitable<void> {
    try {
      auto slot = tracker_.response_loop_sig.slot();
      auto token =
          lib::asio::bind_cancellation_slot(slot, lib::asio::use_awaitable);
      co_await lib::asio::co_spawn(strand_, async_start_internal(), token);
    } catch (const lib::asio::system_error &error) {
      tracker_.response_loop_sig.emit(lib::asio::cancellation_type::all);
    }
  }

  auto async_start_internal() -> lib::asio::awaitable<void> { co_return; }
  auto async_stop() -> lib::asio::awaitable<void> { co_return; }

private:
  detail::client_responser<protocol, websocket_factory, websocket> responser_;
  detail::log log_;
  lib::asio::strand<lib::asio::io_context::executor_type> &strand_;
  detail::task_tracker tracker_;
};
} // namespace detail
} // namespace webpubsub