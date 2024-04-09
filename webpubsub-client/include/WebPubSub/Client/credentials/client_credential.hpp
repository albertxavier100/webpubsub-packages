#ifndef TEST_WEBPUBSUB_CLIENT_CREDENTIAL_HPP
#define TEST_WEBPUBSUB_CLIENT_CREDENTIAL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include <functional>
#include <memory>
#include <string>

namespace webpubsub {
class client_credential {
  using client_access_uri_provider_t =
      std::function<detail::async_t<std::string>()>;

public:
  client_credential(const std::string uri)
      : client_access_uri_provider_(
            [uri = std::move(uri)]() -> detail::async_t<std::string> {
              co_return uri;
            }) {}

  client_credential(
      const client_access_uri_provider_t &client_access_uri_provider)
      : client_access_uri_provider_(client_access_uri_provider) {}

  auto async_get_client_access_uri() const -> detail::async_t<std::string> {
    auto uri = co_await client_access_uri_provider_();
    co_return uri;
  }

private:
  client_access_uri_provider_t client_access_uri_provider_;
};
} // namespace webpubsub
#endif //
