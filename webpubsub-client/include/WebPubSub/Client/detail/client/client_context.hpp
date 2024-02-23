#pragma once
#include "asio/cancellation_signal.hpp"
#include "asio/steady_timer.hpp"
#include "webpubsub/client/detail/async/exclusion_lock.hpp"

namespace webpubsub {
namespace detail {
struct client_signal {
  asio::cancellation_signal listen_loop_cancel_signal;
  asio::cancellation_signal sequence_id_loop_cancel_signal;
};

struct client_finish_notification {
  asio::steady_timer listen_loop_finish_notification;
  asio::steady_timer sequence_id_ack_loop_finish_notification;
};

class client_context {
  exclusion_lock stop_lock_;
};

} // namespace detail
} // namespace webpubsub
