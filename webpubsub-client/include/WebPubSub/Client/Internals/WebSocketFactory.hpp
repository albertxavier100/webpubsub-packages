#pragma once
#include <WebPubSub/Client/Interfaces/IWebSocket.hpp>
#include <WebPubSub/Client/Interfaces/IWebSocketFactory.hpp>

namespace WebPubSub {
class WebSocketFactory : public IWebSocketFactory<WebSocketFactory> {
public:
  WebSocketFactory() = default;
  template <typename TWebSocket>
  TWebSocket create(const std::string &url, const std::string &subProtocol) {
    return TWebSocket(url.c_str(), subProtocol.c_str());
  }
};
} // namespace WebPubSub