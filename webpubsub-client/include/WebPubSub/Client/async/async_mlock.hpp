#pragma once

#include <asio/awaitable.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <asio/experimental/channel.hpp>
#include <asio/io_context.hpp>
#include <asio/use_awaitable.hpp>
#include <memory>

namespace webpubsub {
class async_mlock {
public:
  async_mlock(asio::io_context &io_context) : channel_(io_context, 1) {}

  asio::awaitable<void> async_lock() {
    co_await channel_.async_send(asio::error_code{}, false,
                                 asio::use_awaitable);
  }
  asio::awaitable<void> async_release() {
    co_await channel_.async_receive(asio::use_awaitable);
  }

  void reset() { channel_.reset(); }

private:
  asio::experimental::channel<void(asio::error_code, bool)> channel_;
};
} // namespace webpubsub
