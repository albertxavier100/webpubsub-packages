#pragma once
#include <WebPubSub/Protocols/reliable_json_v1_protocol.hpp>
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>

namespace webpubsub {
template <webpubsub_protocol_t WebPubSubProtocol> struct client_options {
  WebPubSubProtocol protocol;
  bool enable_multi_thread = false;
  bool auto_reconnect = true;
  bool auto_rejoin_groups = true;
  // TODO: change types
  int message_retry_options;
  int reconnect_retry_options;
};
} // namespace webpubsub