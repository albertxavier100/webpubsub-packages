#pragma once
#include <asio/awaitable.hpp>
#include <asio/steady_timer.hpp>
#include <asio/system_error.hpp>
#include <asio/use_awaitable.hpp>

namespace webpubsub {

asio::awaitable<void>
async_delay(const asio::steady_timer::duration &duration) {
  asio::steady_timer timer{co_await asio::this_coro::executor};
  timer.expires_after(duration);
  co_await timer.async_wait(asio::use_awaitable);
}

asio::awaitable<void>
async_timeout(const asio::steady_timer::duration &duration) {
  asio::steady_timer timer{co_await asio::this_coro::executor};
  timer.expires_after(duration);
  co_await timer.async_wait(asio::use_awaitable);
  std::cout << "Timeout!\n";
}

} // namespace webpubsub