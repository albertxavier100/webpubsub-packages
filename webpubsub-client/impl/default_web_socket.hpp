#pragma once
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>
#include <webpubsub/client/concepts/web_socket_t.hpp>

namespace webpubsub {
class default_web_socket {
public:
  default_web_socket(std::string uri, std::string protocol_name,
                     asio::io_context &ioc)
      : ioc_(ioc), uri_(std::move(uri)),
        protocol_name_(std::move(protocol_name)) {}

  default_web_socket &operator=(const default_web_socket &other) {
    if (this != &other) {
      uri_ = other.uri_;
      protocol_name_ = other.protocol_name_;
    }

    return *this;
  }

  asio::awaitable<void> async_connect() {
    co_await test();
  }

  asio::awaitable<void> async_write(const uint64_t *start, const uint64_t size,
                                    const bool as_text = true) {
    co_return;
  }
  // TODO: change status to enum
  asio::awaitable<void> async_read(uint64_t *&start, uint64_t &size,
                                   web_socket_close_status &status) {
    co_return;
  }

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
  std::string uri_;
  std::string protocol_name_;
};

static_assert(web_socket_t<default_web_socket>);
} // namespace webpubsub