#pragma once
#include <asio/awaitable.hpp>
#include <asio/steady_timer.hpp>
#include <asio/system_error.hpp>
#include <asio/use_awaitable.hpp>
#include <webpubsub/client/async/task_cancellation/cancellation_token.hpp>

namespace webpubsub {
asio::awaitable<bool> async_is_coro_cancelled() {
  asio::cancellation_state cs = co_await asio::this_coro::cancellation_state;
  if (cs.cancelled() != asio::cancellation_type::none) {
    std::cout << "start cancelled;\n";
    co_return true;
  }
  co_return false;
}

// TODO: add cancel_signal
asio::awaitable<void>
async_delay(
            const asio::steady_timer::duration &duration) {
  if (co_await async_is_coro_cancelled()) {
    co_return;
  }
  asio::steady_timer timer{co_await asio::this_coro::executor};
  timer.expires_after(duration);
  co_await timer.async_wait(asio::use_awaitable);
}

// TODO: add cancel_signal
asio::awaitable<void>
async_timeout(const asio::steady_timer::duration &duration) {
  if (co_await async_is_coro_cancelled()) {
    co_return;
  }
  asio::steady_timer timer{co_await asio::this_coro::executor};
  timer.expires_after(duration);
  co_await timer.async_wait(asio::use_awaitable);
  std::cout << "Timeout!\n";
  co_return;
}

} // namespace webpubsub