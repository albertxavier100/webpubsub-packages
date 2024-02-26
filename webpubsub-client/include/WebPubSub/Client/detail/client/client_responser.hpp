#pragma once

#include "spdlog/spdlog.h"
#include "webpubsub/client/common/websocket/websocket_close_status.hpp"
#include "webpubsub/client/concepts/websocket_factory_c.hpp"
#include "webpubsub/client/deps/client_responser.hpp"
#include "webpubsub/client/detail/client/task_tracker.hpp"
#include "webpubsub/client/detail/logging/log.hpp"
#include "webpubsub/protocols/webpubsub_protocol_t.hpp"

namespace webpubsub {
namespace detail {
template <webpubsub_protocol_t protocol, typename websocket_factory,
          typename websocket>
  requires websocket_factory_c<websocket_factory, websocket>
class client_responser {
public:
  client_responser(lib::asio::strand<asio::io_context::executor_type> &strand,
                   const bool is_reliable, const task_tracker &tracker,
                   std::shared_ptr<spdlog::logger> logger)
      : strand_(strand), is_reliable_(is_reliable), log_(logger) {}

  auto async_start_response_loop() -> lib::asio::awaitable<void> {
    if (is_reliable_) {
      co_await async_start_sequence_loop();
    }

    websocket_close_status close_status = websocket_close_status::empty;
    try {
      for (;;) {
        auto cs = co_await lib::asio::this_coro::cancellation_state;
        if (cs.cancelled() != lib::asio::cancellation_type::none) {
          break;
        }
        co_await async_read_response();
      }
    } catch (const std::exception &e) {
      log_.failed_to_receive_bytes(e.what());
    };
    /* finally */ {
      try {
        log_.websocket_closed();
        tracker_.sequence_loop_sig.emit(lib::asio::cancellation_type::terminal);
        co_await tracker_.sequence_loop_waiter.async_wait(asio::use_awaitable);
      } catch (...) {
      }
      // TODO: [DEBUG] add back later
      // co_await async_handle_connection_close(websocket_close_status);
      tracker_.response_loop_waiter.cancel();
    };
  }

private:
  // TODO: IMPL
  auto async_read_response() -> lib::asio::awaitable<void> {}

  // TODO: IMPL
  auto async_start_sequence_loop() -> lib::asio::awaitable<void> {}

private:
  lib::asio::strand<asio::io_context::executor_type> &strand_;
  bool is_reliable_;
  log log_;
  protocol protocol_;
  websocket client_;
  task_tracker tracker_;
};
} // namespace detail
} // namespace webpubsub