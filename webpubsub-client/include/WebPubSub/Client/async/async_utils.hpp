#pragma once

#include <asio/awaitable.hpp>
#include <asio/bind_cancellation_slot.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/steady_timer.hpp>
#include <asio/system_error.hpp>
#include <asio/use_awaitable.hpp>
#include <iostream>
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
async_delay(const asio::steady_timer::duration &duration) {
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

void co_spawn_detached_with_signal(
    asio::io_context &io_context, asio::awaitable<void> awaitable,
    asio::cancellation_signal &signal,
    std::function<void(const asio::system_error &)> handle_system_error,
    std::function<void(const std::exception &)> handle_unknown_error) {
  auto run =
      [](asio::io_context &io_context, asio::awaitable<void> awaitable,
         asio::cancellation_signal &signal,
         std::function<void(const asio::system_error &)> handle_system_error,
         std::function<void(const std::exception &)> handle_unknown_error)
      -> asio::awaitable<void> {
    try {
      co_await asio::co_spawn(
          io_context, std::move(awaitable),
          asio::bind_cancellation_slot(signal.slot(), asio::use_awaitable));
    } catch (const asio::system_error &error) {
      std::cout << std::format("Get system error: {}\n", error.what());
      handle_system_error(error);
    } catch (const std::exception &ex) {
      std::cout << std::format("Get unknown exception: {}\n", ex.what());
      handle_unknown_error(ex);
    }
  };
  asio::co_spawn(io_context,
                 run(io_context, std::move(awaitable), signal,
                     std::move(handle_system_error),
                     std::move(handle_unknown_error)),
                 asio::detached);
}

} // namespace webpubsub