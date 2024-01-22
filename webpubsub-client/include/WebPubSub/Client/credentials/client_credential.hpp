#pragma once
#include <functional>
#include <string>

namespace webpubsub {
class client_credential {
public:
  client_credential(const std::string &uri)
      : client_access_uri_provider_([&uri]() -> std::string { return uri; }) {}

  client_credential(
      const std::function<std::string()> &client_access_uri_provider)
      : client_access_uri_provider_(client_access_uri_provider) {}

  std::string getClientAccessUri() { return client_access_uri_provider_(); }

private:
  const std::function<std::string()> client_access_uri_provider_;
};
} // namespace webpubsub