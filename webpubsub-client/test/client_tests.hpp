#pragma once

#include "../impl/default_web_socket_factory.hpp"
#include <WebPubSub/Client/Models/io_service.hpp>
#include <WebPubSub/Client/async/async_utils.hpp>
#include <WebPubSub/Client/client.hpp>
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
  using namespace std::chrono_literals;

  asio::io_context ioc;
  webpubsub::default_web_socket ws(ioc);
  asio::cancellation_signal cancelSignal;
  asio::steady_timer cancelTimer(ioc,
                                 std::chrono::steady_clock::time_point::max());

  asio::co_spawn(ioc, op() || webpubsub::async_delay(3s),
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

TEST(Basic, Asio) {
  using web_socket_factory = webpubsub::default_web_socket_factory<>;
  using webpubsub_client =
      webpubsub::client<web_socket_factory, webpubsub::default_web_socket>;

  webpubsub::reliable_json_v1_protocol p;
  webpubsub::client_credential cre("");
  webpubsub::client_options opts{p};
  web_socket_factory fac;
  webpubsub_client client(opts, cre, fac);
  std::string group("group_name");
  auto &io_context = client.get_io_service().get_io_context();
  webpubsub::cancellation_token_source cts(io_context);
  asio::co_spawn(
      io_context,
      [&client]() -> asio::awaitable<void> { co_await client.async_start(); },
      asio::detached);
}

TEST(Basic, Raw) {
  webpubsub::reliable_json_v1_protocol p;
  webpubsub::client_credential cre("");
  webpubsub::client_options opts{p};
  // webpubsub::client client(opts, cre);
}