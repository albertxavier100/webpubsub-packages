//
// Created by alber on 2024/2/28.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_EVENTS_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_EVENTS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include <variant>

namespace webpubsub {
namespace detail {
// lifetime events
struct to_connecting_state {};
struct to_connected_state {};
struct to_connected_or_stopped_state {};
struct to_disconnected_state {
  std::string connection_id;
  std::string reason;
};
struct to_stopped_state {};
struct to_stopping_state {};
struct to_recovering_or_stopped_state {};
// receive events
// send events

using event_t = std::variant<to_stopped_state, to_connected_state,
                             to_disconnected_state, to_connecting_state,
                             to_recovering_or_stopped_state, to_stopping_state>;
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_EVENTS_HPP
