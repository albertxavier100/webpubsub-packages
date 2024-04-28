//
// Created by alber on 2024/3/5.
//

#ifndef TEST_WEBPUBSUB_CLIENT_SEQUENCE_ID_HPP
#define TEST_WEBPUBSUB_CLIENT_SEQUENCE_ID_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/common/using.hpp"

namespace webpubsub {
namespace detail {
class sequence_id {
public:
  sequence_id(strand_t &strand) : updated_(false), id_(0), lock_(strand) {}

  void reset() {
    updated_ = false;
    id_ = 0;
    lock_.reset();
  }

  auto async_try_get_sequence_id(uint64_t &id) -> async_t<bool> {
    spdlog::trace("lock_.async_send beg");
    co_await lock_.async_lock();
    spdlog::trace("lock_.async_send end");
    if (updated_) {
      id = id_;
      updated_ = true;
      co_await lock_.async_release();
      co_return true;
    }
    spdlog::trace("lock_.async_receive beg");
    co_await lock_.async_release();
    spdlog::trace("lock_.async_receive end");
    id = 0;
    co_return false;
  }

  auto async_try_update(uint64_t id) -> async_t<bool> {
    co_await lock_.async_lock();
    updated_ = true;
    auto changed = false;
    if (id > id_) {
      id_ = id;
      changed = true;
    }
    spdlog::trace("update sid to {0}", id_);
    co_await lock_.async_release();
    co_return changed;
  }

private:
  bool updated_;
  uint64_t id_;
  exclusion_lock lock_;
};
} // namespace detail
} // namespace webpubsub
#endif //
