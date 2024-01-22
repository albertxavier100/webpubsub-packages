#pragma once
namespace webpubsub {
class cancellation_token {
public:
  cancellation_token(asio::steady_timer &timer) : timer_(timer) {}
  std::function<asio::awaitable<void>()> create_async_cancel() {
    return [this]() { return this->async_cancel(); };
  }

private:
  asio::awaitable<void> async_cancel() {
    try {
      co_await timer_.async_wait(asio::use_awaitable);
    } catch (asio::system_error &e) {
      if (e.code() == asio::error::operation_aborted) {
        std::cout << "Canceled\n";
        //  swallow cancel
      } else {
        abort();
      }
    }
  }

private:
  asio::steady_timer &timer_;
};
} // namespace webpubsub