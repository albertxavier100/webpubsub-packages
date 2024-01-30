#pragma once
#include <asio/awaitable.hpp>
#include <asio/steady_timer.hpp>
#include <asio/system_error.hpp>
#include <asio/use_awaitable.hpp>
#include <webpubsub/client/async/task_cancellation/cancellation_token.hpp>

namespace webpubsub {
// TODO: add cancellation token para
asio::awaitable<void>
async_delay(asio::io_context &io_context,
            const asio::steady_timer::duration &duration) {
  asio::steady_timer timer{io_context};
  timer.expires_after(duration);
  co_await timer.async_wait(asio::use_awaitable);
  co_return;
}

asio::awaitable<void>
async_timeout(const asio::steady_timer::duration &duration) {
  asio::steady_timer timer{co_await asio::this_coro::executor};
  timer.expires_after(duration);
  co_await timer.async_wait(asio::use_awaitable);
  std::cout << "Timeout!\n";
  co_return;
}
} // namespace webpubsub