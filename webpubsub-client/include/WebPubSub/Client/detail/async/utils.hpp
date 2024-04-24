#pragma once


#include "webpubsub/client/common/asio.hpp"
#include <iostream>

namespace webpubsub {
namespace detail {
io::awaitable<io::error_code>
async_delay_v2(io::strand<io::io_context::executor_type> &strand,
               const io::steady_timer::duration &duration) {
  io::steady_timer timer{strand, duration};
  auto token = io::as_tuple(io::use_awaitable);

  const auto [ec] = co_await timer.async_wait(token);
  co_return ec;
}
} // namespace detail
} // namespace webpubsub
