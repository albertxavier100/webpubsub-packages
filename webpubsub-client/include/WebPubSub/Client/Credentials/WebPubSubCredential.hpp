#pragma once
#include <WebPubSub/Client/Credentials/WebPubSubCredential.hpp>
#include <functional>
#include <string>

namespace webpubsub {
class client_credential {
public:
  client_credential(const std::string &uri)
      : clientAccessUriProvider([&uri]() -> std::string { return uri; }) {}

  client_credential(const std::function<std::string()> &clientAccessUriProvider)
      : clientAccessUriProvider(clientAccessUriProvider) {}

  std::string getClientAccessUri() { return clientAccessUriProvider(); }

private:
  const std::function<std::string()> clientAccessUriProvider;
};
} // namespace webpubsub