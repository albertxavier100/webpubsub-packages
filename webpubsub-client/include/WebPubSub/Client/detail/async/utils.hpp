#pragma once

#include "asio/awaitable.hpp"
#include "asio/bind_cancellation_slot.hpp"
#include "asio/co_spawn.hpp"
#include "asio/detached.hpp"
#include "asio/steady_timer.hpp"
#include "asio/system_error.hpp"
#include "asio/use_awaitable.hpp"
#include "webpubsub/client/common/asio.hpp"
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
io::awaitable<void> async_delay(io::io_context &io_context,
                                const io::steady_timer::duration &duration) {
  io::steady_timer timer{io_context, duration};
  co_await timer.async_wait(io::use_awaitable);
}

io::awaitable<void>
async_delay(io::strand<io::io_context::executor_type> &strand,
            const io::steady_timer::duration &duration,
            const io::cancellation_slot slot) {
  io::steady_timer timer{strand, duration};
  // auto token =
  //     io::bind_cancellation_slot(start_slot,
  //     io::as_tuple(io::use_awaitable));
  auto token = io::as_tuple(io::use_awaitable);

  const auto [ec] = co_await timer.async_wait(token);
}

io::awaitable<io::error_code>
async_delay_v2(io::strand<io::io_context::executor_type> &strand,
               const io::steady_timer::duration &duration) {
  io::steady_timer timer{strand, duration};
  auto token = io::as_tuple(io::use_awaitable);

  const auto [ec] = co_await timer.async_wait(token);
  co_return ec;
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
