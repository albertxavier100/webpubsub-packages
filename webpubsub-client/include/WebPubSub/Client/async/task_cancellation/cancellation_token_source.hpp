#pragma once

#include <asio/steady_timer.hpp>
#include <initializer_list>

namespace webpubsub {
class cancellation_token_source {
public:
  cancellation_token_source(asio::io_context &io_context)
      : timer_(io_context, std::chrono::steady_clock::time_point::max()) {}


  cancellation_token get_token() { return cancellation_token(timer_); }

  void cancel() {
    timer_.cancel();
    is_cancellation_requested_ = true;
  }
  void cancel_after(const asio::steady_timer::duration &duration) {
    timer_.expires_after(duration);
  }
  bool is_cancellation_requested() { return is_cancellation_requested_; }

private:
  asio::steady_timer timer_;
  bool is_cancellation_requested_ = false;
};
} // namespace webpubsub