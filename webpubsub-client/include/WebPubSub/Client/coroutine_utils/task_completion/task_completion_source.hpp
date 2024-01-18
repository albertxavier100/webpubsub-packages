#pragma once

namespace webpubsub {
template <typename T> class task_completion_source {
public:
  // TODO: remove init_val
  task_completion_source(asio::any_io_executor &exe, T &&init_value)
      : context_(
            new task_completion_context(waiting, std::forward<T>(init_value))),
        timer_(new asio::steady_timer(exe,
                                      asio::steady_timer::time_point::max())) {}
  void cancel() {
    context_->state_ = canceled;
    timer_->cancel();
  }
  void set_value(T value) {
    context_->state = completed;
    context_->value = value;
    timer_->cancel();
  }
  auto get_token() { return task_completion_token<T>(timer_, context_); }

private:
  std::shared_ptr<task_completion_context<T>> context_;
  std::shared_ptr<asio::steady_timer> timer_;
};
} // namespace webpubsub
