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
      : strand_(strand), log_(log), message_loop_coro_completion_(strand_, 1) {}

protected:
  auto
  async_spawn_message_loop_coro(async_t<> async_run,
                                io::cancellation_slot start_slot) -> async_t<> {
    co_await message_loop_coro_completion_.async_send(io::error_code{}, true,
                                                      io::use_awaitable);
    auto &signal = message_loop_coro_cancel_signal_;
    start_slot.assign([&](io::cancellation_type ct) { signal.emit(ct); });

    auto token = io::bind_cancellation_slot(signal.slot(), io::detached);
    io::co_spawn(strand_, std::move(async_run), token);
  }

  auto
  set_lifetime_service(client_lifetime_service<websocket_factory_t, websocket_t>
                           *lifetime_service);

  strand_t &strand_;
  const log &log_;
  client_lifetime_service<websocket_factory_t, websocket_t> *lifetime_;
  io::cancellation_signal message_loop_coro_cancel_signal_;
  notification_t message_loop_coro_completion_;
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
