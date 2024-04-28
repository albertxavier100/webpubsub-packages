#pragma once
#include "spdlog/spdlog.h"
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/config/core.hpp"
#include "webpubsub/client/websocket/default_websocket.hpp"
#include "webpubsub/client/websocket/default_websocket_factory.hpp"
#include "gtest/gtest.h"
#include <cstdlib>

namespace test {
namespace client_with_beast {
TEST(client, with_beast) {
  namespace io = webpubsub::io;
  using namespace io::experimental::awaitable_operators;
  using protocol_t = webpubsub::reliable_json_v1_protocol;
  using options_t = webpubsub::client_options<protocol_t>;
  using factory_t = webpubsub::default_websocket_factory;
  using client_t =
      webpubsub::client<protocol_t, factory_t, webpubsub::default_websocket>;
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
  client.on_group_data.append([](webpubsub::group_data_context context) {
    std::string data;
    context.message.getData<std::string>(data);
    spdlog::trace("received from group: {0}, user: {2}, sid: {1}, data: {3}.",
                  context.message.getGroup(), *context.message.getSequenceId(),
                  *context.message.getFromUserId(), data);
  });

  auto run = [&client]() -> io::awaitable<void> {
    auto group = "ggg";
    auto data = "ddd";
    std::optional<bool> no_echo = false;
    std::optional<DataType> dataType = DataType::Text;

    try {
      auto exe = co_await io::this_coro::executor;
      co_await client.async_start();
      auto const join_res = co_await client.async_join_group(JoinGroupRequest{group});
      co_await io::steady_timer{exe, 1s}.async_wait(io::use_awaitable);
      co_await client.async_send_to_group(
          SendToGroupRequest{group, data, std::nullopt, no_echo, dataType});
      co_await io::steady_timer{exe, 1s}.async_wait(io::use_awaitable);
      co_await client.async_send_to_group(
          SendToGroupRequest{group, data, std::nullopt, no_echo, dataType});
      co_await io::steady_timer{exe, 1s}.async_wait(io::use_awaitable);
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
