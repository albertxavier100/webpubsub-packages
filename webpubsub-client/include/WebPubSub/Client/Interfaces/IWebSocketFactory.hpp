#pragma once
#include <WebPubSub/Client/Interfaces/IWebSocket.hpp>

namespace WebPubSub {
template <class Impl> class IWebSocketFactory {
public:
  template <typename TWebSocket>
  TWebSocket create(const std::string &url, const std::string &subProtocol) {
    static_assert(std::is_base_of<IWebSocket<TWebSocket>, TWebSocket>::value,
                  "TWebSocket does not implement the interface IWebSocket.");

    return static_cast<Impl *>(this)->create(url, subProtocol);
  }
};
} // namespace WebPubSub