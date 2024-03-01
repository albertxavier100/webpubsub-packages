//
// Created by alber on 2024/2/27.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_events.hpp"

namespace webpubsub {
namespace detail {

class ack_cache {};

template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_lifetime_service;

template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
class client_receive_service {

  using client_channel_service_t = detail::client_channel_service;

public:
  client_receive_service(strand &strand,
                         const client_channel_service_t &channel_service,
                         const log &log)
      : strand_(strand), log_(log), channel_service_(channel_service) {}

  // TODO: IMPL
  auto async_spawn_message_loop_coro() -> async_t<> {
    auto &signal = message_loop_cancel_signal_;
    auto sl = (co_await io::this_coro::cancellation_state).slot();
    sl.assign([&](io::cancellation_type ct) {
      spdlog::trace("start slot cancel: {0}", (unsigned int)ct);
      signal.emit(ct);
    });

    struct scope_exit {
      io::cancellation_slot sl;
      ~scope_exit() {
        if (sl.is_connected())
          sl.clear();
      }
    } scope_exit_{sl};

    spdlog::trace("client_receive_service.spawn_message_loop_coro");
    auto token = io::bind_cancellation_slot(signal.slot(), io::detached);
    io::co_spawn(strand_, async_start_message_loop(signal.slot()), token);
  }

  // TODO: IMPL
  auto async_cancel_message_loop_coro() -> async_t<> {
    message_loop_cancel_signal_.emit(io::cancellation_type::terminal);
    // TODO: wait for message loop stop
    co_return;
  }

  // TODO: IMPL
  auto async_wait_ack_id(uint64_t ack_id) -> async_t<> { co_return; }

  auto
  set_lifetime_service(client_lifetime_service<websocket_factory_t, websocket_t>
                           *lifetime_service);

private:
  auto async_start_message_loop(io::cancellation_slot slot) -> async_t<> {
    using namespace std::chrono_literals;
    spdlog::trace("client_receive_service.async_start_message_loop begin");
    bool should_recover = false;
    is_running_receiving_loop_ = true;
    try {
      for (;;) {
        auto cs = co_await io::this_coro::cancellation_state;
        if (cs.cancelled() != io::cancellation_type::none) {
          spdlog::trace("receiving... break");
          break;
        }
        co_await async_delay_v2(strand_, 1s);
        spdlog::trace("receiving...");
      }
    } catch (const std::exception &ex) {
      spdlog::trace("message loop stopped with ex: {0}", ex.what());
      should_recover = true;
      is_running_receiving_loop_ = false;
    }

    if (should_recover) {
      spdlog::trace("async_start_message_loop -- "
                    "lifetime_->async_raise_event -- begin");
      // TODO: still use this slot?
      co_await lifetime_->async_raise_event(to_recovering_state{}, slot);
      co_await lifetime_->async_raise_event(to_connected_state{}, slot);
    }

    co_return;
  }

  ack_cache ack_cache_;
  strand &strand_;
  const client_channel_service_t &channel_service_;
  const log &log_;
  client_lifetime_service<websocket_factory_t, websocket_t> *lifetime_;
  bool is_running_receiving_loop_ = false;
  io::cancellation_signal message_loop_cancel_signal_;
};

#pragma region IMPL
#include "webpubsub/client/detail/services/client_lifetime_service.hpp"
template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
auto client_receive_service<websocket_factory_t, websocket_t>::
    set_lifetime_service(
        client_lifetime_service<websocket_factory_t, websocket_t>
            *lifetime_service) {
  lifetime_ = lifetime_service;
}
#pragma endregion
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_RECEIVE_SERVICE_HPP
