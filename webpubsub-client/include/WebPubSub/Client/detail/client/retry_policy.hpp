//
// Created by alber on 2024/3/7.
//

#ifndef TEST_WEBPUBSUB_CLIENT_RETRY_POLICY_HPP
#define TEST_WEBPUBSUB_CLIENT_RETRY_POLICY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/detail/client/retry_context.hpp"
#include "webpubsub/client/detail/concepts/retry_policy_c.hpp"
#include "webpubsub/client/models/retry_options.hpp"
#include <cmath>
#include <type_traits>

namespace webpubsub {
namespace detail {

class fixed_retry_policy {
public:
  fixed_retry_policy(int max_retry, std::chrono::milliseconds delay)
      : attempts_(0), max_retry_(std::move(max_retry)),
        delay_(std::move(delay)) {}

  auto next_retry_delay() -> std::optional<std::chrono::milliseconds> {
    if (attempts_ > max_retry_) {
      return std::nullopt;
    }
    return delay_;
  }

private:
  const int max_retry_;
  const std::chrono::milliseconds delay_;
  int attempts_;
};
static_assert(retry_policy_c<fixed_retry_policy>);

class exponential_retry_policy {
public:
  exponential_retry_policy(int max_retry, std::chrono::milliseconds delay,
                           std::chrono::milliseconds max_delay)
      : max_retry_(std::move(max_retry)), delay_(std::move(delay)),
        max_delay_(std::move(max_delay)),
        max_retry_to_get_max_delay_(
            ceil(log2(max_delay_.count()) - log2(delay_.count()) + 1)) {}

  auto next_retry_delay() -> std::optional<std::chrono::milliseconds> {
    if (attempts_ > max_retry_) {
      return std::nullopt;
    }

    if (attempts_ >= max_retry_to_get_max_delay_) {
      return max_delay_;
    }

    return delay_ * (1 << (attempts_ - 1));
  }

private:
  const int max_retry_;
  const int max_retry_to_get_max_delay_;
  const std::chrono::milliseconds delay_;
  const std::chrono::milliseconds max_delay_;
  int attempts_;
};

static_assert(retry_policy_c<exponential_retry_policy>);
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_RETRY_POLICY_HPP
