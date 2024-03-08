//
// Created by alber on 2024/3/7.
//

#ifndef TEST_WEBPUBSUB_CLIENT_RETRY_POLICY_HPP
#define TEST_WEBPUBSUB_CLIENT_RETRY_POLICY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/detail/client/retry_context.hpp"
#include "webpubsub/client/models/retry_options.hpp"
#include <type_traits>

namespace webpubsub {
template <typename policy_t>
concept policy_c = requires(policy_t p, detail::retry_context retry) {
  {
    p.next_retry_delay(retry)
  } -> std::same_as<std::optional<std::chrono::milliseconds>>;
};

class fixed_retry_policy {
public:
  fixed_retry_policy(int max_retry, std::chrono::milliseconds delay)
      : max_retry_(std::move(max_retry)), delay_(std::move(delay)) {}

  auto next_retry_delay(detail::retry_context retry)
      -> std::optional<std::chrono::milliseconds> {
    if (retry.attempts > max_retry_) {
      return std::nullopt;
    }
    return delay_;
  }

private:
  int max_retry_;
  std::chrono::milliseconds delay_;
};
static_assert(policy_c<fixed_retry_policy>);

class exponential_retry_policy {
public:
  exponential_retry_policy(int max_retry, std::chrono::milliseconds delay)
      : max_retry_(std::move(max_retry)), delay_(std::move(delay)) {}

  auto next_retry_delay(detail::retry_context retry)
      -> std::optional<std::chrono::milliseconds> {
    if (retry.attempts > max_retry_) {
      return std::nullopt;
    }
  }

private:
  int max_retry_;
  std::chrono::milliseconds delay_;
};

static_assert(policy_c<exponential_retry_policy>);
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_RETRY_POLICY_HPP
