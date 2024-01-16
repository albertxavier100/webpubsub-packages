#pragma once
#include <WebPubSub/Client/Common/Macros.hpp>
#include <WebPubSub/Client/Credentials/WebPubSubCredential.hpp>
#include <WebPubSub/Client/Interfaces/ILatentAction.hpp>
#include <WebPubSub/Client/Interfaces/IWebSocketFactory.hpp>
#include <WebPubSub/Client/Models/WebPubSubClientOptions.hpp>
#include <WebPubSub/Client/Models/WebPubSubGroups.hpp>
#include <WebPubSub/Client/Policies/RetryPolicy.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/IWebPubSubProtocol.hpp>
#include <WebPubSub/Protocols/ReliableJsonV1Protocol.hpp>
#include <asio/awaitable.hpp>
#include <asio/cancellation_signal.hpp>
#include <asio/use_awaitable.hpp>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>

// TODO: put to a new file
namespace WebPubSub {
enum WebPubSubClientState {
  ClientStopped = 0,
  ClientConnecting,
  ClientConnected
};
} // namespace WebPubSub

namespace WebPubSub {
template <typename TProtocol = ReliableJsonV1Protocol> class WebPubSubClient {
public:
  WebPubSubClient(const WebPubSubClientOptions<TProtocol> &options)
      : options(options) {}

  asio::awaitable<void> connectAsync() {}
  // asio::awaitable<RequestResult> joinGroupAsync(const std::string group) {}

private:
private:
  const WebPubSubClientOptions<TProtocol> &options;
};
} // namespace WebPubSub