#pragma once

#include "asio/as_tuple.hpp"
#include "asio/awaitable.hpp"
#include "asio/experimental/awaitable_operators.hpp"
#include "asio/experimental/channel.hpp"
#include "asio/io_context.hpp"
#include "asio/use_awaitable.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include <memory>

namespace webpubsub {
namespace detail {
class exclusion_lock {
public:
  exclusion_lock(strand_t &strand) : channel_(strand, 1) {}

  auto async_lock() -> async_t<> {
    co_await channel_.async_send(io::error_code{}, false, io::use_awaitable);
  }
  auto async_release() -> async_t<> {
    co_await channel_.async_receive(io::use_awaitable);
  }
  auto release() {
    channel_.try_receive([](auto ec, auto a) {});
  }

private:
  io::experimental::channel<void(io::error_code, bool)> channel_;
};
} // namespace detail
} // namespace webpubsub
