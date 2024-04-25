#pragma once
#include "spdlog/spdlog.h"
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/config/core.hpp"
#include "webpubsub/client/websocket/default_websocket.hpp"
#include "gtest/gtest.h"
#include "webpubsub/client/websocket/default_websocket.hpp"
#include "webpubsub/client/websocket/default_websocket_factory.hpp"
#include <cstdlib>

namespace test {
namespace client_with_beast {
TEST(client, with_beast) {
  namespace io = webpubsub::io;
  using namespace io::experimental::awaitable_operators;
  using protocol_t = webpubsub::reliable_json_v1_protocol;
  using options_t = webpubsub::client_options<protocol_t>;
  using factory_t = webpubsub::default_websocket_factory;
  using client_t = webpubsub::client<protocol_t, factory_t, webpubsub::default_websocket>;
  using credential_t = webpubsub::client_credential;
  using namespace std::chrono_literals;
  using strand_t =
      webpubsub::io::strand<webpubsub::io::io_context::executor_type>;

  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto console_logger =
      std::make_shared<spdlog::logger>("console", console_sink);
  if (!spdlog::get("console")) {
    spdlog::register_logger(console_logger);
    spdlog::set_level(spdlog::level::trace);
  }
  webpubsub::io::io_context io_context;
  strand_t strand{io_context.get_executor()};

  factory_t factory;
  protocol_t p;
  options_t opts{p};
  const char *env_key = "WPS_CONN_STR";
  char *env_value = std::getenv(env_key);
  credential_t cre{env_value};
  client_t client(strand, cre, opts, factory, "console");
  client.on_connected.append([](webpubsub::connected_context context) {
    spdlog::trace("connection {0} connected.", context.connection_id);
    });
  
  auto run = [&client]() -> io::awaitable<void> {
    try {
      co_await client.async_start();
      co_await client.async_stop();
      spdlog::trace("test finish");
    } catch (const std::exception &ex) {
      spdlog::trace("get exception in test: {0}", ex.what());
    };
  };
  io::co_spawn(strand, run(), io::detached);
  io_context.run();
}
} // namespace client_with_beast
} // namespace test
