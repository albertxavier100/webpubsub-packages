//
// Created by alber on 2024/2/28.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_STATES_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_STATES_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "client_lifetime_events.hpp"
#include "webpubsub/client/exceptions/exception.hpp"
#include <variant>

namespace webpubsub {
namespace detail {
struct connected {};
struct stopped {};
struct disconnected {};
struct stopping {};
struct recovering {};
struct connecting {};
struct reconnecting {};

using state_t = std::variant<stopped, connecting, connected, recovering,
                             stopping, disconnected, reconnecting>;
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_STATES_HPP
