#pragma once

namespace webpubsub {
enum class task_completion_state {
  waiting,
  completed,
  canceled,
};
} // namespace webpubsub