//
// Created by alber on 2024/4/9.
//

#ifndef TEST_WEBPUBSUB_CLIENT_UTILS_HPP
#define TEST_WEBPUBSUB_CLIENT_UTILS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace webpubsub {
namespace detail {
template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// TODO: test
auto update_delay(retry_context &retry_context) {
  switch (retry_context.retry_mode) {
  case retry_mode::exponential: {
    exponential_retry_policy policy;
    policy.next_retry_delay(retry_context);
  }
  case retry_mode::fixed: {
  default:
    fixed_retry_policy policy;
    policy.next_retry_delay(retry_context);
    break;
  }
  }
  retry_context.attempts++;
}

} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_UTILS_HPP
