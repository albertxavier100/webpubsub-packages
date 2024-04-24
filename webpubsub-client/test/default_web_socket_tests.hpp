#pragma once
#include "default_web_socket.hpp"
#include "spdlog/spdlog.h"
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/config/core.hpp"
#include "gtest/gtest.h"

namespace test {
namespace default_websocket {
TEST(test, default_websocket) {
  namespace io = webpubsub::io;
  io::io_context ioc;
  io::co_spawn(
      ioc,
      []() -> io::awaitable<void> {
        webpubsub::default_web_socket ws("ws://127.0.0.1:8002/",
                                        "json.reliable.webpubsub.azure.v1");
        co_return;
      },
      io::detached);
  ioc.run();
}
} // namespace default_websocket
} // namespace test