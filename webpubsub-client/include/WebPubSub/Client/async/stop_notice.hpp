#pragma once

#include <asio/as_tuple.hpp>
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
    auto atoken = asio::use_awaitable;
    auto token = asio::as_tuple(atoken);
    const auto [ec] = co_await timer_.async_wait(token);
    std::cout << "timer_.async_wait finish" << ec << "\n";
  }

  void stop() { timer_.expires_after(std::chrono::seconds(0)); }

private:
  asio::steady_timer timer_;
};
} // namespace webpubsub
