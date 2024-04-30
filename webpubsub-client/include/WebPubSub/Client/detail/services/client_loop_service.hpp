//
// Created by alber on 2024/3/1.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_LOOP_SERVICE_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_LOOP_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "spdlog/spdlog.h"
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/client/retry_policy.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/logging/log.hpp"

namespace webpubsub {
namespace detail {
class client_loop_service {
  using channel_t = io::experimental::channel<void(io::error_code, bool)>;

public:
  client_loop_service(const std::string name, strand_t &strand, const log &log)
      : strand_(strand), loop_track_(strand, 1), log_(log),
        name_(std::move(name)) {
    loop_track_.close();
  }

  auto spawn_loop_coro(async_t<> async_run) {
    loop_track_.reset();
    auto token = io::bind_cancellation_slot(
        inner_cancel_signal_.slot(),
        [&name_ = this->name_, &loop_track = this->loop_track_](auto ep) {
          spdlog::trace("{0} coro finish", name_);
          if (loop_track.is_open()) {
            auto _ = loop_track.try_send(io::error_code{}, true);
          }
        });
    io::co_spawn(strand_, std::move(async_run), std::move(token)

    );
  }

  auto reset() -> void {
    loop_track_.cancel();
    loop_track_.reset();
    loop_track_.close();
  }

  auto async_cancel_loop_coro() -> async_t<> {
    spdlog::trace("{0} cancel emit", name_);
    inner_cancel_signal_.emit(io::cancellation_type::terminal);
    spdlog::trace("{0} wait loop finish begin", name_);
    if (loop_track_.is_open()) {
      co_await loop_track_.async_receive(io::use_awaitable);
    } else {
      spdlog::trace("{0} loop is already stopped before wait", name_);
    }
    spdlog::trace("{0} wait loop finish end", name_);
    co_return;
  }

  auto &strand() { return strand_; }
  auto &log() { return log_; }

private:
  strand_t &strand_;
  const detail::log &log_;
  const std::string name_;
  io::cancellation_signal inner_cancel_signal_;
  channel_t loop_track_;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LOOP_SERVICE_HPP
