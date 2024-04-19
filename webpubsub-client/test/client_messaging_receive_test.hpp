//
// Created by alber on 2024/4/11.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_MESSAGING_RECEIVE_TEST_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_MESSAGING_RECEIVE_TEST_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "spdlog/spdlog.h"
#include "webpubsub/client/detail/services/client_receive_service.hpp"
#include "gtest/gtest.h"

namespace webpubsub {
namespace detail {

// TODO: impl
TEST(messaging, callbacks) {
  using namespace webpubsub::detail;
  using strand_t = io::strand<webpubsub::io::io_context::executor_type>;
  using protocol_t = reliable_json_v1_protocol;

  io::io_context ioc;
  strand_t strand(ioc.get_executor());
  client_options<protocol_t> options;
  std::unordered_map<uint64_t, ack_entity> ack_cache;
  log log("console");

  client_receive_service<protocol_t> receive(strand, options, ack_cache, log);

  io::co_spawn(ioc, []() -> async_t<> {}, [](auto e) {});
  ioc.run();
}
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_CLIENT_MESSAGING_RECEIVE_TEST_HPP
