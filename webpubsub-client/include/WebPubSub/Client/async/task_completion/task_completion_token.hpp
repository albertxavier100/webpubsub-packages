#pragma once

namespace webpubsub {
template <typename T> class task_completion_token {
  class task_completion_source;

public:
  task_completion_token() = default;
  // TODO: use ref is enough
  task_completion_token(std::shared_ptr<asio::steady_timer> timer,
                        std::shared_ptr<task_completion_context<T>> result)
      : timer_(timer), context_(result) {}
  asio::awaitable<void> async_wait() {
    try {
      co_await timer_->async_wait(asio::use_awaitable);
    } catch (boost::system::system_error &e) {
      if (e.code() == asio::error::operation_aborted) {
        std::cout << "Task completed or cancelled\n";
      }
    }
  }
  std::shared_ptr<task_completion_context<T>> get_context() { return context_; }

private:
  std::shared_ptr<task_completion_context<T>> context_;
  std::shared_ptr<asio::steady_timer> timer_;
};
} // namespace webpubsub
