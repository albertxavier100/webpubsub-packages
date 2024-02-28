//
// Created by alber on 2024/2/28.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_STATE_VISITOR_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_STATE_VISITOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <variant>

namespace webpubsub {
namespace detail {
template <typename websocket_factory_t, typename websocket_t>
  requires websocket_factory_c<websocket_factory_t, websocket_t>
struct state_visitor {

  using async_move_to_t =
      auto(event_t,
           client_lifetime_service<websocket_factory_t, websocket_t>*)
      -> async_t<state_t>;

  // TODO: IMPL
  void operator()(stopped &s) {
    async_move_to = std::bind(
        &stopped::async_move_to<state_t, websocket_factory_t, websocket_t>, &s,
        std::placeholders::_1, std::placeholders::_2);
  };
  void operator()(connecting &s) {
    async_move_to = std::bind(
        &connecting::async_move_to<state_t, websocket_factory_t, websocket_t>,
        &s, std::placeholders::_1, std::placeholders::_2);
  };
  // TODO: IMPL
  void operator()(connected &s) {};
  // TODO: IMPL
  void operator()(recovering &s) {};
  // TODO: IMPL
  void operator()(stopping &s) {};
  std::function<async_move_to_t> async_move_to;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_LIFETIME_STATE_VISITOR_HPP
