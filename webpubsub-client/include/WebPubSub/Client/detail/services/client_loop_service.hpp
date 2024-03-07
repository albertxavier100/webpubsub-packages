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
#include "webpubsub/client/concepts/websocket_factory_c.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/logging/log.hpp"

namespace webpubsub {
namespace detail {

template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_lifetime_service;

template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_loop_service {
public:
  client_loop_service(strand_t &strand, const log &log)
      : strand_(strand), log_(log) {}

  auto spawn_loop_coro(async_t<> async_run, io::cancellation_slot start_slot) {
    auto &signal = cancel_signal_;
    start_slot.assign([&](io::cancellation_type ct) { signal.emit(ct); });

    auto token = io::bind_cancellation_slot(signal.slot(), io::detached);
    io::co_spawn(strand_, std::move(async_run), token);
    spdlog::trace("loop started");
  }

  auto async_cancel_loop_coro() -> async_t<> {
    cancel_signal_.emit(io::cancellation_type::terminal);
    spdlog::trace("cancel finished");
    co_return;
  }

  auto
  set_lifetime_service(client_lifetime_service<websocket_factory_t, websocket_t>
                           *lifetime_service);

  auto lifetime() { return lifetime_; }
  auto &strand() { return strand_; }
  auto &log() { return log_; }
  auto &cancel_signal() { return cancel_signal_; }

private:
  strand_t &strand_;
  const detail::log &log_;
  client_lifetime_service<websocket_factory_t, websocket_t> *lifetime_;
  io::cancellation_signal cancel_signal_;
};

#pragma region IMPL
#include "webpubsub/client/detail/services/client_lifetime_service.hpp"
template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
auto client_loop_service<websocket_factory_t, websocket_t>::
    set_lifetime_service(
        client_lifetime_service<websocket_factory_t, websocket_t>
            *lifetime_service) {
  lifetime_ = lifetime_service;
}
#pragma endregion
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LOOP_SERVICE_HPP
