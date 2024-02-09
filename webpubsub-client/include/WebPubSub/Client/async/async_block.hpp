#pragma once

#include <asio/awaitable.hpp>
#include <asio/bind_cancellation_slot.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <asio/experimental/channel.hpp>
#include <asio/redirect_error.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>
#include <chrono>
#include <optional>

namespace webpubsub {
// Helper class for try-catch-finally blocks that support co_await in catch and
template <typename TException> class async_block {
public:
  static asio::awaitable<void> async_run(
      std::function<asio::awaitable<void>()> try_block,
      std::function<asio::awaitable<void>(const TException &)> catch_block,
      std::function<asio::awaitable<void>()> finally_block) {
    std::optional<TException> exception_opt;
    try {
      co_await try_block();
    } catch (const TException &e) {
      exception_opt = e;
    }
    co_await async_run_catched(exception_opt, catch_block, finally_block);
  }

private:
  static asio::awaitable<void> async_run_catched(
      const std::optional<TException> &exception_opt,
      std::function<asio::awaitable<void>(const TException &)> catch_block,
      std::function<asio::awaitable<void>()> finally_block) {
    std::optional<TException> new_exception_opt;
    if (!exception_opt) {
      co_await finally_block();
      co_return;
    }

    try {
      co_await catch_block(*exception_opt);
    } catch (const TException &e) {
      new_exception_opt = e;
    }
    co_await finally_block();
    if (new_exception_opt) {
      throw *new_exception_opt;
    }
  }
};
} // namespace webpubsub
