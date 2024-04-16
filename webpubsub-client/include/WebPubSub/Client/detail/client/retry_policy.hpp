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
  fixed_retry_policy() {}

  auto next_retry_delay(retry_context &retry_context) {
    if (retry_context.attempts > retry_context.max_retry) {
      retry_context.delay = std::nullopt;
    }
    retry_context.attempts++;
  }

private:
};
static_assert(retry_policy_c<fixed_retry_policy>);

class exponential_retry_policy {
public:
  exponential_retry_policy() {}

  auto next_retry_delay(retry_context &retry_context) {
    if (retry_context.attempts > retry_context.max_retry) {
      retry_context.delay = std::nullopt;
    }
    if (retry_context.delay >= retry_context.max_delay) {
      retry_context.attempts++;
      retry_context.delay = retry_context.max_delay;
    }
    retry_context.attempts++;
    retry_context.delay = *retry_context.delay + *retry_context.delay;
  }
};

static_assert(retry_policy_c<exponential_retry_policy>);

using retry_policies_t =
    std::variant<fixed_retry_policy, exponential_retry_policy>;
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_RETRY_POLICY_HPP
