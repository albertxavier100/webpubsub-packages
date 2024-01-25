#pragma once
#include <webpubsub/client/async/task_completion/task_completion_context.hpp>
#include <webpubsub/client/async/task_completion/task_completion_state.hpp>

namespace webpubsub {
template <typename T> class task_completion_source {
public:
  // TODO: remove init_val
  task_completion_source(asio::any_io_executor &exe, T &&init_value)
      : context_(new task_completion_context(task_completion_state::waiting,
                                             std::forward<T>(init_value))),
        timer_(new asio::steady_timer(exe,
                                      asio::steady_timer::time_point::max())) {}
  void cancel() {
    context_->state_ = task_completion_state::canceled;
    timer_->cancel();
  }
  bool set_value_once(T value) {
    if (is_set_value_) {
      return false;
    }
    context_->state = task_completion_state::completed;
    context_->value = value;
    timer_->cancel();
    is_set_value_ = true;
    return true;
  }
  auto get_token() { return task_completion_token<T>(timer_, context_); }

private:
  std::shared_ptr<task_completion_context<T>> context_;
  std::shared_ptr<asio::steady_timer> timer_;

  bool is_set_value_ = false;
};
} // namespace webpubsub