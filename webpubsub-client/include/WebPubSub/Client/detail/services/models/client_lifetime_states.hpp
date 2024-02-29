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

// TODO: add concept for states
namespace webpubsub {
namespace detail {
struct connected {};

struct connecting {};

struct stopped {};
struct stopping {};
struct recovering {};

using state_t =
    std::variant<stopped, connecting, connected, recovering, stopping>;
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_STATES_HPP
