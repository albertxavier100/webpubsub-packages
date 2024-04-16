//
// Created by alber on 2024/3/8.
//

#ifndef TEST_WEBPUBSUB_CLIENT_RETRY_POLICY_C_HPP
#define TEST_WEBPUBSUB_CLIENT_RETRY_POLICY_C_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <chrono>
#include <type_traits>

namespace webpubsub {
namespace detail {
template <typename policy_t>
concept retry_policy_c = requires(policy_t p, retry_context &retry_context) {
  { p.next_retry_delay(retry_context) } -> std::same_as<void>;
};
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_RETRY_POLICY_C_HPP
