//
// Created by alber on 2024/3/7.
//

#ifndef TEST_WEBPUBSUB_CLIENT_LOOP_TRACKER_HPP
#define TEST_WEBPUBSUB_CLIENT_LOOP_TRACKER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "spdlog/spdlog.h"
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/exceptions/exception.hpp"

namespace webpubsub {
namespace detail {
class loop_tracker {
  using channel_t = io::experimental::channel<void(io::error_code, bool)>;

public:
  loop_tracker(strand_t &strand)
      : strand_(strand), channel_(strand, 1), is_waiting_(false) {}

  auto finish(const std::string &loop_name) {
    if (!channel_.is_open()) {
      throw invalid_operation(loop_name + " channel is already closed");
    }
    if (!is_waiting_) {
      spdlog::trace("{0} is not being waited", loop_name);
      return;
    }
    auto ok = channel_.try_send(io::error_code{}, true);
    spdlog::trace("{0} try send finish", loop_name);
  }

  auto async_wait() -> async_t<> {
    spdlog::trace("loop tracker.async_wait.is_waiting_ = {0}", is_waiting_);
    if (!channel_.is_open()) {
      co_return;
    }
    is_waiting_ = true;
    co_await channel_.async_receive(io::use_awaitable);
    spdlog::trace("loop tracker.async_wait close channel");
    channel_.close();
  }

  auto reset() -> void { channel_.reset(); }

private:
  channel_t channel_;
  strand_t &strand_;
  bool is_waiting_;
};
} // namespace detail
} // namespace webpubsub
#endif
