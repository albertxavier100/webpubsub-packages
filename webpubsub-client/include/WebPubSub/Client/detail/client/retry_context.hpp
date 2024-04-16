//
// Created by alber on 2024/3/7.
//

#ifndef TEST_WEBPUBSUB_CLIENT_RETRY_CONTEXT_HPP
#define TEST_WEBPUBSUB_CLIENT_RETRY_CONTEXT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/models/retry_mode.hpp"

namespace webpubsub {
namespace detail {
struct retry_context {
  // TODO: make them const?
  std::chrono::milliseconds max_delay;
  uint64_t max_retry;
  retry_mode retry_mode;
  //
  int attempts;
  std::optional<std::chrono::milliseconds> delay;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_RETRY_CONTEXT_HPP
