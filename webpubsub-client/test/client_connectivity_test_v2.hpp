#pragma once

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "webpubsub/client/client_v2.hpp"
#include "webpubsub/client/detail/async/utils.hpp"
#include "gtest/gtest.h"

webpubsub::io::io_context io_context;
webpubsub::io::strand<webpubsub::io::io_context::executor_type> strand{
    io_context.get_executor()};

class test_websocket_1 {
  template <typename T = void> using async_t = webpubsub::io::awaitable<T>;

public:
  test_websocket_1(){};

  virtual auto async_connect() -> async_t<> { co_return; };
  virtual auto async_close() -> async_t<> { co_return; };

  virtual auto async_write(const std::string payload) -> async_t<> {
    co_return;
  };
  virtual auto
  async_read(std::string &payload,
             webpubsub::websocket_close_status &status) -> async_t<> {
    using namespace std::chrono_literals;

    co_await webpubsub::detail::async_delay(strand, 1s);
    if (!is_connected_) {
      co_await async_read_connected_message(payload, status);
      is_connected_ = true;
    }
  };

private:
  auto async_read_connected_message(std::string &payload,
                                    webpubsub::websocket_close_status &status)
      -> async_t<> {
    status = webpubsub::websocket_close_status::empty;
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

private:
  bool is_connected_ = false;
};
static_assert(webpubsub::websocket_c<test_websocket_1>);

template <typename websocket> class test_websocket_factory_1 {
public:
  std::unique_ptr<websocket> create(std::string uri,
                                    std::string protocol_name) {
    return std::make_unique<websocket>();
  }
};
static_assert(webpubsub::websocket_factory_c<
              test_websocket_factory_1<test_websocket_1>, test_websocket_1>);

TEST(connectivity, happy_start_stop) {
  using protocol_t = webpubsub::reliable_json_v1_protocol;
  using options_t = webpubsub::client_options<protocol_t>;
  using factory_t = test_websocket_factory_1<test_websocket_1>;
  using client_t =
      webpubsub::client_v2<protocol_t, factory_t, test_websocket_1>;

  spdlog::set_level(spdlog::level::trace);

  factory_t factory;
  protocol_t p;
  options_t opts{p};
  client_t client(strand, opts, factory, "console");

  auto async_test = [&]() -> webpubsub::io::awaitable<void> {
    co_await client.async_start();
  };
  webpubsub::io::co_spawn(strand, async_test(), webpubsub::io::detached);
  io_context.run();
}
