#pragma once
#include <WebPubSub/Protocols/reliable_json_v1_protocol.hpp>
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>

#include "webpubsub/client/models/retry_options.hpp"

namespace webpubsub {
template <webpubsub_protocol_t protocol_t> struct client_options {
  protocol_t protocol;
  bool enable_multi_thread = false;
  bool auto_reconnect = true;
  bool auto_rejoin_groups = true;
  retry_options message_retry_options;
  retry_options reconnect_retry_options;
  // TODO: find a best one
  size_t max_buffer_size = 10000;
};
} // namespace webpubsub