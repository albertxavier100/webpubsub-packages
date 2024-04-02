//
// Created by alber on 2024/3/1.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_LOOP_SERVICE_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_LOOP_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "client_lifetime_service.hpp"
#include "spdlog/spdlog.h"
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/client/retry_policy.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/logging/log.hpp"

namespace webpubsub {
namespace detail {
class client_loop_service {
public:
  client_loop_service(strand_t &strand, const log &log)
      : strand_(strand), loop_tracker_(strand), log_(log) {}

  auto spawn_loop_coro(async_t<> async_run, io::cancellation_slot start_slot) {
    auto &signal = cancel_signal_;
    start_slot.assign([&](io::cancellation_type ct) { signal.emit(ct); });

    auto token = io::bind_cancellation_slot(signal.slot(), io::detached);
    io::co_spawn(strand_, std::move(async_run), token);
  }

  auto async_cancel_loop_coro() -> async_t<> {
    spdlog::trace("cancel finished");
    cancel_signal_.emit(io::cancellation_type::terminal);
    spdlog::trace("wait receive loop finish begin");
    co_await loop_tracker_.async_wait();
    spdlog::trace("wait receive loop finish end");
    co_return;
  }

  auto async_start_loop(async_t<> start_loop_operation) -> async_t<> {
    struct exit_scope {
      ~exit_scope() {
        spdlog::trace("loop finished beg");
        lt_.finish();
      }
      loop_tracker &lt_;
    } _{loop_tracker_};

    co_await std::move(start_loop_operation);
  }

  auto &strand() { return strand_; }
  auto &log() { return log_; }
  auto &cancel_signal() { return cancel_signal_; }

private:
  strand_t &strand_;
  const detail::log &log_;
  io::cancellation_signal cancel_signal_;
  loop_tracker loop_tracker_;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LOOP_SERVICE_HPP
