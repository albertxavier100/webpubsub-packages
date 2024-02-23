#pragma once

#include <asio/awaitable.hpp>
#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>
#include <memory>

namespace webpubsub {
class stop_notice {
public:
  stop_notice(asio::io_context &io_context)
      : timer_(io_context, asio::steady_timer::time_point::max()) {}

  asio::awaitable<void> async_wait() {
    std::cout << "timer_.async_wait start\n";
    try {
      co_await timer_.async_wait(asio::use_awaitable);
    } catch (...) {
      std::cout << "hello???\n";
    }
    std::cout << "something stop\n";
  }

  void stop() {
    timer_.expires_after(std::chrono::seconds(0));
    timer_.cancel();
  }

private:
  asio::steady_timer timer_;
};
} // namespace webpubsub
