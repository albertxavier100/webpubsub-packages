#pragma once
#include <asio/experimental/awaitable_operators.hpp>
#include <asio/experimental/channel.hpp>
#include <memory>

namespace webpubsub {
class operation_waiter {
public:
  operation_waiter(asio::io_context &io_context) : channel_(io_context, 1) {}

  asio::awaitable<void> async_start() {
    co_await channel_.async_send(asio::error_code{}, false,
                                 asio::use_awaitable);
  }

  asio::awaitable<void> async_complete() {
    co_await channel_.async_receive(asio::use_awaitable);
  }

  asio::awaitable<void> async_wait() {
    co_await channel_.async_send(asio::error_code{}, false,
                                 asio::use_awaitable);
    co_await channel_.async_receive(asio::use_awaitable);
  }

  void reset() { channel_.reset(); }

private:
  asio::experimental::channel<void(asio::error_code, bool)> channel_;
};
} // namespace webpubsub
