//
// Created by alber on 2024/3/7.
//

#ifndef TEST_WEBPUBSUB_CLIENT_RETRY_OPTIONS_HPP
#define TEST_WEBPUBSUB_CLIENT_RETRY_OPTIONS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/models/retry_mode.hpp"
#include <chrono>

namespace webpubsub {
struct retry_options {
  uint64_t max_retry;
  std::chrono::milliseconds delay;
  std::chrono::milliseconds max_delay;
  retry_mode retry_mode;
};
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_RETRY_OPTIONS_HPP
