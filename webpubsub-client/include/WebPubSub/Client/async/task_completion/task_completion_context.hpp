#pragma once

#include <webpubsub/client/async/task_completion/task_completion_state.hpp>

namespace webpubsub {
template <typename T> struct task_completion_context {
  task_completion_context(task_completion_state state, T &&value)
      : state(state), value(std::forward<T>(value)) {}
  task_completion_state state;
  T value;
};
} // namespace webpubsub