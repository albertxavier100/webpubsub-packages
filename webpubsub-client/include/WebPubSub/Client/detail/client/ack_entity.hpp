//
// Created by alber on 2024/3/5.
//

#ifndef TEST_WEBPUBSUB_CLIENT_ACK_ENTITY_HPP
#define TEST_WEBPUBSUB_CLIENT_ACK_ENTITY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/exceptions/exception.hpp"
#include <variant>

namespace webpubsub {
namespace detail {
class ack_entity {
  enum class result {
    cancelled,
    completed,
  };
  using result_t = std::variant<invalid_operation, result>;
  using ack_channel_t =
      io::experimental::channel<void(io::error_code, result_t)>;

public:
  ack_entity(strand_t &strand, const uint64_t &ack_id)
      : id_(ack_id), ack_channel_(strand, 1) {}

  auto async_finish_with(result_t result) -> async_t<> {
    co_await ack_channel_.async_send(io::error_code{}, std::move(result),
                                     io::use_awaitable);
  }

  auto async_await() -> async_t<result_t> {
    auto result = co_await ack_channel_.async_receive(io::use_awaitable);
    co_return result;
  }

private:
  uint64_t id_;
  ack_channel_t ack_channel_;
};
} // namespace detail
} // namespace webpubsub
#endif
