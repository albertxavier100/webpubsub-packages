#pragma once
#include "webpubsub/client/websocket/default_websocket.hpp"
#include "spdlog/spdlog.h"
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/config/core.hpp"
#include "gtest/gtest.h"

namespace test {
namespace default_websocket {
TEST(test, default_websocket) {
  GTEST_SKIP() << "Skipping this test \n";

  namespace io = webpubsub::io;
  io::io_context ioc;
  io::co_spawn(
      ioc,
      []() -> io::awaitable<void> {
        webpubsub::default_websocket ws("ws://127.0.0.1:8002/",
                                         "json.reliable.webpubsub.azure.v1");
        co_await ws.async_connect();
        std::string read;
        close_code_t code;
        co_await ws.async_read(read, code);
        std::cout << "read = " << read << "\n";
        for (auto i = 0; i < 10; i++) {
          co_await ws.async_write("hahaha");
          co_await io::steady_timer{co_await io::this_coro::executor,
                                    std::chrono::seconds(1)}
              .async_wait(io::use_awaitable);
        }
        co_await ws.async_close();
        co_return;
      },
      io::detached);
  ioc.run();
}
} // namespace default_websocket
} // namespace test