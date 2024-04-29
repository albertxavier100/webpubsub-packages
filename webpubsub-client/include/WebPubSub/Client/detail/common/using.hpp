//
// Created by alber on 2024/2/27.
//

#ifndef TEST_WEBPUBSUB_CLIENT_USING_HPP
#define TEST_WEBPUBSUB_CLIENT_USING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/config/core.hpp"

namespace webpubsub {
namespace detail {
template <typename t = void> using async_t = io::awaitable<t>;
using strand_t = io::strand<io::io_context::executor_type>;
using lock_t = io::experimental::channel<void(io::error_code, bool)>;
using close_code_t = uint16_t;
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_USING_HPP
