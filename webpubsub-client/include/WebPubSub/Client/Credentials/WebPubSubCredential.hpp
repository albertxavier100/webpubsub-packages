#pragma once
#include <WebPubSub/Client/Credentials/WebPubSubCredential.hpp>
#include <functional>
#include <string>

namespace WebPubSub {
class WebPubSubCredential {

public:
  WebPubSubCredential(const std::string &uri)
      : clientAccessUriProvider([&uri]() -> std::string { return uri; }) {}

  WebPubSubCredential(
      const std::function<std::string()> &clientAccessUriProvider)
      : clientAccessUriProvider(clientAccessUriProvider) {}

  std::string getClientAccessUri() { return clientAccessUriProvider(); }

private:
  const std::function<std::string()> clientAccessUriProvider;
};
} // namespace WebPubSub