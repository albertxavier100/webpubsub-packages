#pragma once
#include <WebPubSub/Protocols/IWebPubSubProtocol.hpp>
#include <WebPubSub/Protocols/ReliableJsonV1Protocol.hpp>

namespace webpubsub {
template <typename TWebPubSubProtocol> struct client_options {
  static_assert(std::is_base_of<IWebPubSubProtocol<TWebPubSubProtocol>,
                                TWebPubSubProtocol>::value,
                "TWebPubSubProtocol does not implement the interface "
                "IWebPubSubProtocol.");

  TWebPubSubProtocol protocol;
  bool enable_multi_thread = false;
  bool auto_reconnect = true;
  bool auto_rejoin_groups = true;
  // TODO: change types
  int message_retry_options;
  int reconnect_retry_options;
};
} // namespace webpubsub