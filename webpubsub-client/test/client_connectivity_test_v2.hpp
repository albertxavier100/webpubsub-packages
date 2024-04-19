#pragma once

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "webpubsub/client/client_v2.hpp"
#include "webpubsub/client/detail/async/utils.hpp"
#include "webpubsub/client/models/retry_mode.hpp"
#include "gtest/gtest.h"

namespace test {
namespace connectivity {

using strand_t =
    webpubsub::io::strand<webpubsub::io::io_context::executor_type>;
webpubsub::io::io_context io_context;
strand_t strand{io_context.get_executor()};
template <typename T = void> using async_t = webpubsub::io::awaitable<T>;

auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
auto console_logger = std::make_shared<spdlog::logger>("console", console_sink);

class test_websocket_1 {

public:
  template <typename T = void> using async_t = webpubsub::io::awaitable<T>;
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

bool thrown_exception = false;
class test_websocket_reconnect : public test_websocket_1 {
public:
  auto
  async_read(std::string &payload,
             webpubsub::websocket_close_status &status) -> async_t<> override {
    using namespace std::chrono_literals;
    spdlog::trace("already thrown_exception = {0}", thrown_exception);
    if (!thrown_exception) {
      thrown_exception = true;
      throw std::exception("test reconnect");
    }
    co_await webpubsub::detail::async_delay_v2(strand, 1s);
  };
};
static_assert(webpubsub::websocket_c<test_websocket_reconnect>);

template <typename websocket> class test_websocket_factory_1 {
public:
  std::unique_ptr<websocket> create(std::string uri,
                                    std::string protocol_name) {
    return std::make_unique<websocket>();
  }
};
static_assert(webpubsub::websocket_factory_c<
              test_websocket_factory_1<test_websocket_1>, test_websocket_1>);

TEST(connectivity, start_stop_basic) {
  namespace io = webpubsub::io;
  using namespace io::experimental::awaitable_operators;
  using protocol_t = webpubsub::reliable_json_v1_protocol;
  using options_t = webpubsub::client_options<protocol_t>;
  using factory_t = test_websocket_factory_1<test_websocket_1>;
  using client_t =
      webpubsub::client_v2<protocol_t, factory_t, test_websocket_1>;
  using credential_t = webpubsub::client_credential;
  using namespace std::chrono_literals;

  if (!spdlog::get("console")) {
    spdlog::register_logger(console_logger);
    spdlog::set_level(spdlog::level::trace);
  }

  factory_t factory;
  protocol_t p;
  options_t opts{p};
  credential_t cre{"abcd"};
  client_t client(strand, cre, opts, factory, "console");

  spdlog::trace("start test");
  auto async_test = [&]() -> async_t<> {
    try {
      co_await client.async_start();
      spdlog::trace("client started in test");
      co_await client.async_stop();
      spdlog::trace("client stopped in test");
      spdlog::trace("test finish");
    } catch (const std::exception &ex) {
      spdlog::trace("get exception in async_test: {0}", ex.what());
    };
    co_return;
  };

  io::co_spawn(strand, async_test(), io::detached);
  io_context.run();
}

TEST(connectivity, start_stop_with_cancel) {
  namespace io = webpubsub::io;
  using namespace io::experimental::awaitable_operators;
  using protocol_t = webpubsub::reliable_json_v1_protocol;
  using options_t = webpubsub::client_options<protocol_t>;
  using factory_t = test_websocket_factory_1<test_websocket_1>;
  using credential_t = webpubsub::client_credential;
  using client_t =
      webpubsub::client_v2<protocol_t, factory_t, test_websocket_1>;
  using namespace std::chrono_literals;

  if (!spdlog::get("console")) {
    spdlog::register_logger(console_logger);
    spdlog::set_level(spdlog::level::trace);
  }

  factory_t factory;
  protocol_t p;
  options_t opts{p};
  credential_t cre{"abcd"};
  client_t client(strand, cre, opts, factory, "console");

  spdlog::trace("start test");
  auto async_test = [&]() -> async_t<> {
    try {
      co_await client.async_start();
      spdlog::trace("client connected in test");
    } catch (...) {
      spdlog::trace("xxx");
    };
    co_return;
  };

  auto async_cancel_2s = [&]() -> async_t<> {
    using namespace std::chrono_literals;
    co_await webpubsub::detail::async_delay_v2(strand, 3s);
    spdlog::trace("emit cancel");
    co_await client.async_cancel();
    spdlog::trace("finish cancel");
  };
  try {
    io::co_spawn(strand, async_test(), io::detached);
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

TEST(connectivity, auto_reconnect) {
  namespace io = webpubsub::io;
  using namespace io::experimental::awaitable_operators;
  using protocol_t = webpubsub::reliable_json_v1_protocol;
  using options_t = webpubsub::client_options<protocol_t>;
  using factory_t = test_websocket_factory_1<test_websocket_reconnect>;
  using credential_t = webpubsub::client_credential;
  using client_t =
      webpubsub::client_v2<protocol_t, factory_t, test_websocket_reconnect>;
  using namespace std::chrono_literals;

  if (!spdlog::get("console")) {
    spdlog::register_logger(console_logger);
    spdlog::set_level(spdlog::level::trace);
  }

  factory_t factory;
  protocol_t p;
  options_t opts{
      .protocol = p,
      .reconnect_retry_options = {.max_retry = 3,
                                  .delay = std::chrono::milliseconds(1000),
                                  .retry_mode = webpubsub::retry_mode::fixed}};
  credential_t cre{"abcd"};
  client_t client(strand, cre, opts, factory, "console");

  spdlog::trace("start test");

  auto async_test = [&]() -> async_t<> {
    using namespace std::chrono_literals;
    try {
      co_await client.async_start();
      spdlog::trace("client started in test");
      // TODO: improve this test
      co_await webpubsub::detail::async_delay_v2(strand, 2s);
      co_await client.async_stop();
      spdlog::trace("client stopped in test");
    } catch (const std::exception &ex) {
      spdlog::trace("get exception in async_test: {0}", ex.what());
    };
    co_return;
  };

  io::co_spawn(strand, async_test(), io::detached);

  io_context.run();
}

// TODO: add test that can be stop by cancel signal after reconnect or recover

} // namespace connectivity
} // namespace test
