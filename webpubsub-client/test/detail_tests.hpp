//
// Created by alber on 2024/3/6.
//

#ifndef TEST_WEBPUBSUB_CLIENT_DETAIL_TESTS_HPP
#define TEST_WEBPUBSUB_CLIENT_DETAIL_TESTS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "spdlog/spdlog.h"
#include "webpubsub/client/detail/client/ack_entity.hpp"
#include "gtest/gtest.h"

using namespace webpubsub;
using namespace webpubsub::detail;

using strand_t = io::strand<io::io_context::executor_type>;

namespace test {
namespace detail {

io::io_context io_context;
strand_t strand{io_context.get_executor()};

TEST(detail, ack_entity) {
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

} // namespace detail
} // namespace test

#endif // TEST_WEBPUBSUB_CLIENT_DETAIL_TESTS_HPP
