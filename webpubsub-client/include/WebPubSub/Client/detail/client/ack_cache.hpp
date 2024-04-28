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

  ack_cache(strand_t &strand) : lock_(strand), strand_(strand) {}

  auto async_add_or_get(uint64_t id) -> async_t<> {
    co_await lock_.async_send(io::error_code{}, true, io::use_awaitable);
    cache_.try_emplace(id, ack_channel_t{strand_, 1});
    co_await lock_.async_receive(io::use_awaitable);
  }
  auto async_finish(uint64_t id, result_t result) -> async_t<> {
    co_await lock_.async_send(io::error_code{}, true, io::use_awaitable);
    if (cache_.contains(id)) {
      spdlog::trace("finish ack {0}.", id);
      cache_.at(id).try_send(io::error_code{}, result);
      cache_.erase(id);
    }
    co_await lock_.async_receive(io::use_awaitable);
  }

  auto async_wait(uint64_t id) -> async_t<result_t> {
    co_await lock_.async_send(io::error_code{}, true, io::use_awaitable);
    auto res = co_await cache_.at(id).async_receive(io::use_awaitable);
    co_await lock_.async_receive(io::use_awaitable);
    co_return res;
  }

  auto async_finish_all(result_t result) -> async_t<> {
    co_await lock_.async_send(io::error_code{}, true, io::use_awaitable);
    for (auto &kv : cache_) {
      kv.second.try_send(io::error_code{}, result);
    }
    co_await lock_.async_receive(io::use_awaitable);
  }

  auto async_reset() -> async_t<> {
    co_await lock_.async_send(io::error_code{}, true, io::use_awaitable);
    for (auto &kv : cache_) {
      kv.second.reset();
    }
    cache_.clear();
    co_await lock_.async_receive(io::use_awaitable);
  }

private:
  std::unordered_map<uint64_t, ack_channel_t> cache_;
  lock_t lock_;
  strand_t strand_;
};
} // namespace detail
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_ACK_CACHE_HPP
