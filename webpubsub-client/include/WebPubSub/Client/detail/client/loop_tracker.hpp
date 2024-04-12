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
// TODO: re-impl
class loop_tracker {
  using channel_t = io::experimental::channel<void(io::error_code, bool)>;

  enum class loop_state {
    before_waiting,
    running,
    waiting,
    finished,
  };

public:
  loop_tracker(strand_t &strand)
      : strand_(strand), channel_(strand, 1), is_waiting_(false),
        state_(loop_state::before_waiting) {}

  auto start() { state_ = loop_state::running; }
  auto finish(const std::string &loop_name) {
    if (!channel_.is_open()) {
      throw invalid_operation(loop_name + " channel is already closed");
    }

    auto ok = channel_.try_send(io::error_code{}, true);
    spdlog::trace("{0} try send finish", loop_name);
  }

  auto async_wait() -> async_t<> {
    spdlog::trace("loop tracker.async_wait.state = {0}", (int)state_);
    if (state_ != loop_state::waiting) {
      spdlog::trace("loop is not in waiting state. state = {0}", (int)state_);
      co_return;
    }
    if (!channel_.is_open()) {
      spdlog::trace("loop is not open");
      state_ = loop_state::finished;
      co_return;
    }
    spdlog::trace("loop tracker channel_.async_receive");
    // TODO: what if loop is already finish before waiting
    co_await channel_.async_receive(io::use_awaitable);
    spdlog::trace("loop tracker.async_wait close channel");
    channel_.close();
  }

  auto reset() -> void { channel_.reset(); }

private:
  channel_t channel_;
  strand_t &strand_;
  bool is_waiting_;
  loop_state state_;
};
} // namespace detail
} // namespace webpubsub
#endif
