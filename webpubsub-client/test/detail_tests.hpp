//
// Created by alber on 2024/3/6.
//

#ifndef TEST_WEBPUBSUB_CLIENT_DETAIL_TESTS_HPP
#define TEST_WEBPUBSUB_CLIENT_DETAIL_TESTS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "spdlog/spdlog.h"
#include "webpubsub/client/credentials/client_credential.hpp"
#include "webpubsub/client/detail/client/ack_entity.hpp"
#include "webpubsub/client/detail/client/sequence_id.hpp"
#include "gtest/gtest.h"

using namespace webpubsub;
using namespace webpubsub::detail;

using strand_t = io::strand<io::io_context::executor_type>;

namespace test {
namespace detail {

TEST(detail, ack_entity) {
  io::io_context io_context;
  strand_t strand{io_context.get_executor()};

  uint64_t id;
  ack_entity ack{strand, id};

  auto finish = [&]() -> async_t<> {
    co_await ack.async_finish_with(ack_entity::result::cancelled);
  };
  auto wait = [&]() -> async_t<> {
    auto res = co_await ack.async_wait();
    EXPECT_EQ(std::get<ack_entity::result>(res), ack_entity::result::cancelled);
  };
  io::co_spawn(io_context, finish(), io::detached);
  io::co_spawn(io_context, wait(), io::detached);
  io_context.run();
}

// TODO: impl
TEST(detail, sequence_id) {}

// TODO: add test for simple std::string
TEST(detail, credential) {
  using namespace webpubsub;
  io::io_context io_context;
  std::string uri_1;
  auto op_1 = [&uri_1]() -> io::awaitable<void> {
    client_credential cre{
        []() -> io::awaitable<std::string> { co_return "abcd"; }};
    uri_1 = co_await cre.async_get_client_access_uri();
    co_return;
  };

  std::string uri_2;
  auto op_2 = [&uri_2]() -> io::awaitable<void> {
    client_credential cre{"xyzq"};
    uri_2 = co_await cre.async_get_client_access_uri();
    co_return;
  };

  io::co_spawn(io_context, std::move(op_1()), io::detached);
  io::co_spawn(io_context, std::move(op_2()), io::detached);
  io_context.run();

  ASSERT_EQ(uri_1, "abcd");
  ASSERT_EQ(uri_2, "xyzq");
}
} // namespace detail
} // namespace test

#endif // TEST_WEBPUBSUB_CLIENT_DETAIL_TESTS_HPP
