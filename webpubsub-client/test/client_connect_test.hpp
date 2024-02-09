#pragma once
#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
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
#include <format>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <webpubsub/client/async/async_utils.hpp>
#include <webpubsub/client/client.hpp>
#include <webpubsub/client/models/io_service.hpp>

class test_web_socket_base {
public:
  test_web_socket_base(){};

  virtual asio::awaitable<void> async_connect() { co_return; };
  virtual asio::awaitable<void> async_close() { co_return; };

  virtual asio::awaitable<void> async_write(const std::string payload) {
    co_return;
  };
  virtual asio::awaitable<void>
  async_read(std::string &payload, webpubsub::web_socket_close_status &status) {
    co_await async_read_connected_message(payload, status);
  };

private:
  asio::awaitable<void>
  async_read_connected_message(std::string &payload,
                               webpubsub::web_socket_close_status &status) {
    status = webpubsub::web_socket_close_status::empty;
    using namespace std::chrono_literals;
    auto json = R"(
{
    "type": "system",
    "event": "connected",
    "userId": "user1",
    "connectionId": "abcdefghijklmnop",
    "reconnectionToken": "<token>"
}
    )"_json;
    payload = json.dump();
    co_return;
  }
};
static_assert(webpubsub::web_socket_t<test_web_socket_base>);

template <typename TWebSocket> class test_web_socket_factory {
public:
  std::unique_ptr<TWebSocket> create(std::string uri,
                                     std::string protocol_name) {
    return std::make_unique<TWebSocket>();
  }
};
static_assert(
    webpubsub::web_socket_factory_t<
        test_web_socket_factory<test_web_socket_base>, test_web_socket_base>);

// TODO: better interface
TEST(RAW, Asio) {
  using webpubsub_client =
      webpubsub::client<test_web_socket_factory<test_web_socket_base>,
                        test_web_socket_base,
                        webpubsub::reliable_json_v1_protocol>;

  // client setup
  webpubsub::reliable_json_v1_protocol p;
  webpubsub::client_credential cre("");
  webpubsub::client_options opts{p};
  webpubsub::io_service io_service;
  test_web_socket_factory<test_web_socket_base> fac;
  webpubsub_client client(opts, cre, fac, io_service);

  client.on_connected.append([](webpubsub::connected_context context) {
    std::cout << std::format("@@@@@ begin validate client.start: ");
    EXPECT_EQ("abcdefghijklmnop", context.connection_id);
    EXPECT_EQ("user1", context.user_id);
    EXPECT_EQ("<token>", context.reconnection_token);
  });

  std::string group("group_name");
  auto &io_context = client.get_io_service().get_io_context();

  auto run = [&]() -> asio::awaitable<void> {
    asio::cancellation_signal cs_start;
    co_await asio::co_spawn(
        io_context, client.async_start(),
        asio::bind_cancellation_slot(cs_start.slot(), asio::use_awaitable));
  };

  asio::co_spawn(io_context, run(), asio::detached);

  io_context.run();
}