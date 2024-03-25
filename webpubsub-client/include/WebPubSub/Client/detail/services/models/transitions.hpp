//
// Created by alber on 2024/2/29.
//

#ifndef TEST_WEBPUBSUB_CLIENT_TRANSITIONS_HPP
#define TEST_WEBPUBSUB_CLIENT_TRANSITIONS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/concepts/websocket_factory_c.hpp"
#include "webpubsub/client/detail/async/utils.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/detail/concepts/transition_context_c.hpp"
#include "webpubsub/client/detail/logging/log.hpp"
#include "webpubsub/client/detail/services/client_lifetime_service.hpp"
#include "webpubsub/client/detail/services/models/client_lifetime_states.hpp"
#include "webpubsub/client/exceptions/exception.hpp"

// source states
#include "webpubsub/client/detail/services/transitions/connected.hpp"
#include "webpubsub/client/detail/services/transitions/connecting.hpp"
#include "webpubsub/client/detail/services/transitions/disconnected.hpp"
#include "webpubsub/client/detail/services/transitions/recovering.hpp"
#include "webpubsub/client/detail/services/transitions/stopped.hpp"
#include "webpubsub/client/detail/services/transitions/stopping.hpp"

namespace webpubsub {
namespace detail {
// TODO: careful about un-wired transitions
template <transition_context_c transition_context_t>
auto async_on_event(transition_context_t *context, auto &state,
                    auto &event) -> async_t<state_t> {
  throw std::logic_error{std::format(":::Transition::: Unsupported state and "
                                     "transition: state: {}, transition: {}",
                                     typeid(state).name(),
                                     typeid(event).name())};
}

template <transition_context_c transition_context_t>
auto async_on_enter(transition_context_t *context, auto &state,
                    auto &event) -> async_t<> {
  co_return;
}

} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_TRANSITIONS_HPP
