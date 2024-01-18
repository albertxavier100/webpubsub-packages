#pragma once

#include "../impl/DefaultWebSocket.hpp"
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>
#include <gtest/gtest.h>

TEST(WebPubSubClient, Basic) {
  asio::io_context ioc;
  webpubsub::DefaultWebSocket ws(ioc);
  asio::co_spawn(ioc, ws.connectAsync(), [](std::exception_ptr e) {
    if (e)
      try {
        std::rethrow_exception(e);
      } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
      }
  });

  ioc.run();
}