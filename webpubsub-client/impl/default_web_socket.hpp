#pragma once
#include <WebPubSub/Client/concepts/web_socket_t.hpp>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>

namespace webpubsub {
class default_web_socket {
public:
  default_web_socket(asio::io_context &ioc) : ioc_(ioc) {}
  asio::awaitable<void> async_connect() { co_await test(); }

private:
  asio::awaitable<void> test() {
    using namespace std::chrono_literals;
    asio::steady_timer timer(ioc_);
    timer.expires_after(1s);
    std::cout << "start delay" << std::endl;
    co_await timer.async_wait(asio::use_awaitable);
    std::cout << "stop delay" << std::endl;
  }

private:
  asio::io_context &ioc_;
};

static_assert(web_socket_t<default_web_socket>,
              "default_web_socket doesn't implement web_socket_t");
} // namespace webpubsub