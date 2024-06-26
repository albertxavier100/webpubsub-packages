//
// Created by alber on 2024/4/18.
//

#ifndef TEST_WEBPUBSUB_CLIENT_ACK_CACHE_HPP
#define TEST_WEBPUBSUB_CLIENT_ACK_CACHE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include "webpubsub/client/exceptions/exception.hpp"
#include "webpubsub/client/models/request_result.hpp"
#include <variant>

namespace webpubsub {
namespace detail {
class ack_cache {
public:
  enum class result {
    cancelled,
  };

  using result_t = std::variant<invalid_operation, result, request_result>;
  using ack_channel_t =
      io::experimental::channel<void(io::error_code, result_t)>;

  ack_cache() {}

  auto add_or_get(strand_t & strand, uint64_t id) {
    cache_.try_emplace(id, ack_channel_t{strand, 1});
  }
  auto finish(uint64_t id, result_t result) {
    if (cache_.contains(id)) {
      spdlog::trace("finish ack {0}.", id);
      cache_.at(id).try_send(io::error_code{}, result);
      cache_.erase(id);
    }
  }

  auto async_wait(uint64_t id) -> async_t<result_t> {
    auto res = co_await cache_.at(id).async_receive(io::use_awaitable);
    co_return res;
  }

  auto finish_all(result_t result) {
    for (auto &kv : cache_) {
      kv.second.try_send(io::error_code{}, result);
    }
  }

  auto reset() {
    for (auto &kv : cache_) {
      kv.second.cancel();
      kv.second.reset();
    }
    cache_.clear();
  }

private:
  std::unordered_map<uint64_t, ack_channel_t> cache_;
};
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_ACK_CACHE_HPP
