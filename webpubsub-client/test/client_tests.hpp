#pragma once

// #include "../impl/default_web_socket_factory.hpp"
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
#include <webpubsub/client/async/async_utils.hpp>
#include <webpubsub/client/client.hpp>
#include <webpubsub/client/models/io_service.hpp>

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

//// TODO: move to a new file
// TEST(Utils, Cancel) {
//   using namespace asio::experimental::awaitable_operators;
//   using namespace std::chrono_literals;
//
//   asio::io_context ioc;
//   std::string uri;
//   std::string protocol_name;
//   webpubsub::default_web_socket ws(uri, protocol_name, ioc);
//   asio::cancellation_signal cancelSignal;
//   asio::steady_timer cancelTimer(ioc,
//                                  std::chrono::steady_clock::time_point::max());
//
//   asio::co_spawn(ioc, op() || webpubsub::async_delay(ioc, 3s),
//                  [](std::exception_ptr e, auto a) {
//                    if (e)
//                      try {
//                        std::rethrow_exception(e);
//                      } catch (std::exception &e) {
//                        std::cerr << "Error: " << e.what() << "\n";
//                      }
//                  });
//
//   asio::co_spawn(ioc, stopAfter4500Milliseconds(cancelTimer),
//   asio::detached);
//
//   ioc.run();
// }

class test_web_socket {
public:
  test_web_socket() = default;

  asio::awaitable<void> async_connect(
      const std::optional<webpubsub::cancellation_token> &cancellation_token =
          std::nullopt) {
    co_return;
  };

  asio::awaitable<void> async_write(
      const uint64_t *write_ptr, const uint64_t write_size,
      const bool write_as_text = true,
      const std::optional<webpubsub::cancellation_token> &cancellation_token =
          std::nullopt) {
    co_return;
  };
  asio::awaitable<void> async_read(
      uint64_t *&read_ptr, uint64_t &read_size,
      webpubsub::web_socket_close_status &status,
      const std::optional<webpubsub::cancellation_token> &cancellation_token =
          std::nullopt) {
    co_return;
  };
};
static_assert(webpubsub::web_socket_t<test_web_socket>);

class test_web_socket_factory {
public:
  std::unique_ptr<test_web_socket> create(std::string uri,
                                          std::string protocol_name) {
    return std::make_unique<test_web_socket>();
  }
};
static_assert(
    webpubsub::web_socket_factory_t<test_web_socket_factory, test_web_socket>);

TEST(Basic, Asio) {
  using webpubsub_client =
      webpubsub::client<test_web_socket_factory, test_web_socket,
                        webpubsub::reliable_json_v1_protocol>;

  webpubsub::reliable_json_v1_protocol p;
  webpubsub::client_credential cre("");
  webpubsub::client_options opts{p};
  webpubsub::io_service io_service;
  test_web_socket_factory fac;
  webpubsub_client client(opts, cre, fac, io_service);
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