//
// Created by alber on 2024/2/24.
//

#ifndef TEST_WEBPUBSUB_CLIENT_ASIO_HPP
#define TEST_WEBPUBSUB_CLIENT_ASIO_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#ifdef ASIO_STANDALONE
#include <asio/version.hpp>

#if (ASIO_VERSION / 100000) == 1 && ((ASIO_VERSION / 100) % 1000) < 8
static_assert(false, "The minimum version of standalone Asio is 1.8.0");
#endif

#include <asio/awaitable.hpp>
#include <asio/bind_cancellation_slot.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/experimental/channel.hpp>
#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>
#include <asio/strand.hpp>
#include <asio/use_awaitable.hpp>

#else
#include <boost/asio/awaitable.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/error_code.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/beast/ssl.hpp>

#endif

#include "webpubsub/client/config/core.hpp"

#endif // TEST_WEBPUBSUB_CLIENT_ASIO_HPP
