#pragma once
#include <WebPubSub/Client/Interfaces/IWebSocket.hpp>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>

namespace WebPubSub {
class DefaultWebSocket {
public:
  DefaultWebSocket(asio::io_context &ioc) : ioc(ioc) {}
  asio::awaitable<void> connectAsync() { co_await test(); }

private:
  asio::awaitable<void> test() {
    using namespace std::chrono_literals;
    asio::steady_timer timer(ioc);
    timer.expires_after(1s);
    std::cout << "start delay" << std::endl;
    co_await timer.async_wait(asio::use_awaitable);
    std::cout << "stop delay" << std::endl;
  }

private:
  asio::io_context &ioc;
};

static_assert(IWebSocket<DefaultWebSocket>);
} // namespace WebPubSub