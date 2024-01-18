#pragma once
namespace webpubsub {
class cancellation_token_source {
public:
  cancellation_token_source(asio::any_io_executor &exe)
      : timer_(exe, std::chrono::steady_clock::time_point::max()) {}
  cancellation_token get_token() { return cancellation_token(timer_); }
  void cancel() {
    timer_.cancel();
    is_cancellation_requested_ = true;
  }
  bool is_cancellation_requested() { return is_cancellation_requested_; }

private:
  asio::steady_timer timer_;
  bool is_cancellation_requested_ = false;
};
} // namespace webpubsub