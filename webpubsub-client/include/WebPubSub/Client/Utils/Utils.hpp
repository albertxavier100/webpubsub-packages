#pragma once
#include <asio/awaitable.hpp>
#include <asio/steady_timer.hpp>
#include <asio/system_error.hpp>
#include <asio/use_awaitable.hpp>

namespace WebPubSub {
asio::awaitable<void> cancel(asio::steady_timer &timer) {
  try {
    co_await timer.async_wait(asio::use_awaitable);
  } catch (asio::system_error &e) {
    if (e.code() == asio::error::operation_aborted) {
      std::cout << "Canceled\n";
      //  swallow cancel
    } else {
      abort();
    }
  }
}
} // namespace WebPubSub