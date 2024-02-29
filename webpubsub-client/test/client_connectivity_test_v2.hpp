#pragma once

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "webpubsub/client/client_v2.hpp"
#include "webpubsub/client/detail/async/utils.hpp"
#include "gtest/gtest.h"

using strand_t =
    webpubsub::io::strand<webpubsub::io::io_context::executor_type>;
webpubsub::io::io_context io_context;
strand_t strand{io_context.get_executor()};
template <typename T = void> using async_t = webpubsub::io::awaitable<T>;

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

    co_await webpubsub::detail::async_delay(strand, 1s, cancel_.slot());
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

  webpubsub::io::cancellation_signal cancel_;
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
  namespace io = webpubsub::io;
  using namespace io::experimental::awaitable_operators;
  using protocol_t = webpubsub::reliable_json_v1_protocol;
  using options_t = webpubsub::client_options<protocol_t>;
  using factory_t = test_websocket_factory_1<test_websocket_1>;
  using client_t =
      webpubsub::client_v2<protocol_t, factory_t, test_websocket_1>;
  using namespace std::chrono_literals;

  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto console_logger =
      std::make_shared<spdlog::logger>("console", console_sink);
  spdlog::register_logger(console_logger);
  spdlog::set_level(spdlog::level::trace);

  factory_t factory;
  protocol_t p;
  options_t opts{p};
  client_t client(strand, opts, factory, "console");

  asio::cancellation_signal cancel;
  asio::cancellation_signal cancel_dummy;

  spdlog::trace("start test");
  auto async_test = [&]() -> async_t<> {
    try {
      co_await client.async_start(std::move(cancel.slot()));
      spdlog::trace("client connected in test");
    } catch (...) {
      spdlog::trace("xxx");
    };
    co_return;
  };

  auto async_cancel_2s = [&]() -> async_t<> {
    using namespace std::chrono_literals;
    co_await webpubsub::detail::async_delay(strand, 1s, cancel_dummy.slot());
    spdlog::trace("emit cancel");
    cancel.emit(io::cancellation_type::terminal);
  };
  try {
    auto token = io::bind_cancellation_slot(cancel.slot(), io::detached);
    io::co_spawn(strand, async_test(), token);
    io::co_spawn(strand, async_cancel_2s(), io::detached);
  } catch (...) {
    spdlog::trace("ex in test body");
  }
  try {
    io_context.run();
  } catch (std::exception &ex) {
    spdlog::trace("ex in main: {0}", ex.what());
  }
}
