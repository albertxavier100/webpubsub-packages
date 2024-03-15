//
// Created by alber on 2024/3/14.
//

#ifndef TEST_WEBPUBSUB_CLIENT_RETRY_POLICY_FACTORY_HPP
#define TEST_WEBPUBSUB_CLIENT_RETRY_POLICY_FACTORY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/detail/client/retry_policy.hpp"
#include "webpubsub/client/detail/concepts/retry_policy_c.hpp"
#include "webpubsub/client/models/retry_options.hpp"
#include <type_traits>

namespace webpubsub {
namespace detail {
template <retry_policy_c retry_policy_t> class retry_policy_factory {
public:
  static auto create(const retry_options &o) -> retry_policy_t {
    switch (o.retry_mode) {

    case retry_mode::exponential:
      return exponential_retry_policy(o.max_retry, o.delay, o.max_delay);
    case retry_mode::fixed:
    default:
      return fixed_retry_policy(o.max_retry, o.delay);
    }
  }
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_RETRY_POLICY_FACTORY_HPP
