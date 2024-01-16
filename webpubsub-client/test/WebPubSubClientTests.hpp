#pragma once

#include "../impl/DefaultWebSocket.hpp"
#include <WebPubSub/Client/Utils/Utils.hpp>
#include <WebPubSub/Client/WebPubSubClient.hpp>
#include <asio/awaitable.hpp>
#include <asio/bind_cancellation_slot.hpp>
#include <asio/cancellation_signal.hpp>
#include <asio/cancellation_state.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>
#include <chrono>
#include <gtest/gtest.h>

auto op() -> asio::awaitable<void> {
  using namespace std::chrono_literals;

  while (true) {
    std::cout << "Working..." << std::endl;
    asio::steady_timer timer{co_await asio::this_coro::executor};
    timer.expires_after(1s);
    co_await timer.async_wait(asio::use_awaitable);
  }
}

asio::awaitable<void> stopAfter4500Milliseconds(asio::steady_timer &timer) {
  asio::steady_timer stopAfterFiveSeconds{co_await asio::this_coro::executor};
  stopAfterFiveSeconds.expires_after(std::chrono::milliseconds{4500});
  co_await stopAfterFiveSeconds.async_wait(asio::use_awaitable);
  timer.cancel();
}

// TODO: move to a new file
TEST(Utils, Cancel) {
  using namespace asio::experimental::awaitable_operators;

  asio::io_context ioc;
  WebPubSub::DefaultWebSocket ws(ioc);
  asio::cancellation_signal cancelSignal;
  asio::steady_timer cancelTimer(ioc,
                                 std::chrono::steady_clock::time_point::max());

  asio::co_spawn(ioc, op() || WebPubSub::cancel(cancelTimer),
                 [](std::exception_ptr e, auto a) {
                   if (e)
                     try {
                       std::rethrow_exception(e);
                     } catch (std::exception &e) {
                       std::cerr << "Error: " << e.what() << "\n";
                     }
                 });

  asio::co_spawn(ioc, stopAfter4500Milliseconds(cancelTimer), asio::detached);

  ioc.run();
}