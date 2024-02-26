#pragma once

#include "webpubsub/client/deps/task_tracker.hpp"

namespace webpubsub {
namespace detail {
struct task_tracker {
  lib::asio::cancellation_signal response_loop_sig;
  lib::asio::cancellation_signal sequence_loop_sig;
  lib::asio::steady_timer response_loop_waiter;
  lib::asio::steady_timer sequence_loop_waiter;
};
} // namespace detail
} // namespace webpubsub