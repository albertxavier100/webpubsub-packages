#pragma once
#include <WebPubSub/Protocols/IWebPubSubProtocol.hpp>

namespace webpubsub {
template <typename TWebPubSubProtocol> struct WebPubSubClientOptions {
  static_assert(std::is_base_of<IWebPubSubProtocol<TWebPubSubProtocol>,
                                TWebPubSubProtocol>::value,
                "TWebPubSubProtocol does not implement the interface "
                "IWebPubSubProtocol.");

public:
  TWebPubSubProtocol protocol;
  bool autoReconnect;
  bool autoRejoinGroups;
  // TODO: change types
  int messageRetryOptions;
  int reconnectRetryOptions;
};
} // namespace webpubsub
