#pragma once
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <concepts>
#include <type_traits>

namespace webpubsub {
template <typename F>
concept ScopeGuardFinallyBlock = requires(F finally_block) { finally_block(); };

template <ScopeGuardFinallyBlock FinallyBlock> class scope_guard {
public:
  scope_guard(asio::io_context &io_context, FinallyBlock finally_block)
      : io_context_(io_context), finally_block_(std::move(finally_block)) {}
  ~scope_guard() {
    if constexpr (std::is_same_v<asio::awaitable<void>,
                                 decltype(finally_block_())>) {
      asio::co_spawn(io_context_, finally_block_(), asio::detached);
    } else {
      finally_block_();
    }
  }
  scope_guard() = delete;
  scope_guard(const scope_guard &) = delete;
  scope_guard &operator=(const scope_guard &) = delete;
  scope_guard(scope_guard &&rhs) = delete;

protected:
  asio::io_context &io_context_;
  FinallyBlock finally_block_;
};
} // namespace webpubsub