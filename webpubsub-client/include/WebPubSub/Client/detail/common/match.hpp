//
// Created by alber on 2024/2/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_STATE_MATCH_HPP
#define TEST_WEBPUBSUB_CLIENT_STATE_MATCH_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <variant>

namespace webpubsub {
namespace detail {
template <class... Ts> struct async_overloaded : Ts... {
  using Ts::operator()...;
};

template <class... Ts> async_overloaded(Ts...) -> async_overloaded<Ts...>;

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};

template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <typename Variant, typename... Matchers>
auto match(Variant &&variant, Matchers &&...matchers) {
  return std::visit(detail::overloaded{std::forward<Matchers>(matchers)...},
                    std::forward<Variant>(variant));
}
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_STATE_MATCH_HPP
