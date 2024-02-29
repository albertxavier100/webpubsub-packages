//
// Created by alber on 2024/2/28.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_CHANNEL_SERVICE_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_CHANNEL_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/logging/log.hpp"
#include "webpubsub/client/detail/services/models/events/client_lifetime_events.hpp"

namespace webpubsub {
namespace detail {

template <typename data_t = uint8_t>
using channel_t = io::experimental::channel<void(io::error_code, data_t)>;

class client_channel_service {
public:
  client_channel_service(strand &strand, const detail::log &log)
      : log_{log}, lifetime_channel_{strand}, receive_channel_{strand},
        send_channel_{strand} {}

  auto &get_lifetime_channel() { return lifetime_channel_; }
  auto &get_receive_channel() { return receive_channel_; }
  auto &get_send_channel() { return send_channel_; }

private:
  const log &log_;
  channel_t<event_t> lifetime_channel_;
  channel_t<> receive_channel_;
  channel_t<> send_channel_;
};

} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_CHANNEL_SERVICE_HPP
