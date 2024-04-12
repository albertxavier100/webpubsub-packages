//
// Created by alber on 2024/4/11.
//

#ifndef TEST_WEBPUBSUB_CLIENT_FAILED_CONNECTION_CONTEXT_HPP
#define TEST_WEBPUBSUB_CLIENT_FAILED_CONNECTION_CONTEXT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <optional>
#include <string>

namespace webpubsub {
namespace detail {
struct failed_connection_context {
  bool should_recover;
  std::optional<std::string> reconnect_uri;
};
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_FAILED_CONNECTION_CONTEXT_HPP
