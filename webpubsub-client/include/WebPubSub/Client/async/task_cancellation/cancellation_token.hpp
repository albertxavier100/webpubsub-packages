#pragma once
#include <asio/awaitable.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>

namespace webpubsub {
class cancellation_token {
public:
  cancellation_token(asio::steady_timer &timer)
      : timer_(timer), cancelled_(false) {}

  asio::awaitable<void> async_cancel() const {
    try {
      co_await timer_.async_wait(asio::use_awaitable);
    } catch (asio::system_error &e) {
      // TODO: should cancel
      // cancelled_ = true;
      if (e.code() == asio::error::operation_aborted) {
        std::cout << "Canceled\n";
        //  swallow cancel
      } else {
        abort();
      }
    }
  }

  asio::awaitable<bool> is_cancelled() const {
    co_await asio::post(asio::use_awaitable);
    co_return cancelled_;
  }

private:
  asio::steady_timer &timer_;
  bool cancelled_;
};
} // namespace webpubsub