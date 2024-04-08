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
  client_loop_service(const std::string name, strand_t &strand, const log &log)
      : strand_(strand), loop_tracker_(strand), log_(log),
        name_(std::move(name)) {}

  auto spawn_loop_coro(async_t<> async_run) {
    auto token =
        io::bind_cancellation_slot(inner_cancel_signal_.slot(), io::detached);
    io::co_spawn(strand_, std::move(async_start_loop(std::move(async_run))),
                 std::move(token)

    );
  }

  auto reset() -> void { loop_tracker_.reset(); }

  auto async_cancel_loop_coro() -> async_t<> {
    spdlog::trace("{0} cancel emit", name_);
    inner_cancel_signal_.emit(io::cancellation_type::terminal);
    spdlog::trace("{0} wait loop finish begin", name_);
    co_await loop_tracker_.async_wait();
    spdlog::trace("{0} wait loop finish end", name_);
    co_return;
  }

  auto async_start_loop(async_t<> start_loop_operation) -> async_t<> {
    struct exit_scope {
      ~exit_scope() {
        spdlog::trace("{0} loop finished", name_);
        lt_.finish(name_);
      }
      loop_tracker &lt_;
      const std::string &name_;
    } _{loop_tracker_, name_};
    co_await std::move(start_loop_operation);
  }

  auto &strand() { return strand_; }
  auto &log() { return log_; }

private:
  strand_t &strand_;
  const detail::log &log_;
  loop_tracker loop_tracker_;
  const std::string name_;
  io::cancellation_signal inner_cancel_signal_;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LOOP_SERVICE_HPP
