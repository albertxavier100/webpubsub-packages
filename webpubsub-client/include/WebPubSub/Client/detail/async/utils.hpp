#pragma once

#include "asio/awaitable.hpp"
#include "asio/bind_cancellation_slot.hpp"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "asio/steady_timer.hpp"
#include "asio/system_error.hpp"
#include "asio/use_awaitable.hpp"
#include <iostream>

namespace webpubsub {
namespace detail {
asio::awaitable<bool> async_is_coro_cancelled() {
  asio::cancellation_state cs = co_await asio::this_coro::cancellation_state;
  if (cs.cancelled() != asio::cancellation_type::none) {
    std::cout << "coro cancelled;\n";
    co_return true;
  }
  co_return false;
}

// TODO: add cancel_signal
asio::awaitable<void>
async_delay(asio::io_context &io_context,
            const asio::steady_timer::duration &duration) {
  asio::steady_timer timer{io_context, duration};
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
} // namespace detail
} // namespace webpubsub
