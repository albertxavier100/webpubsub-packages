#pragma once
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/detail/common/using.hpp"
#include <iostream>
#include <optional>

namespace webpubsub {

// TODO: impl this, so that uint* and char* don't need to cast
template <typename T>
concept websocket_c =
    requires(T t, std::string write_frame, std::string &read_frame,
             uint16_t &close_code) {
      { t.async_connect() } -> std::same_as<io::awaitable<void>>;
      { t.async_close() } -> std::same_as<io::awaitable<void>>;
      { t.async_write(write_frame) } -> std::same_as<io::awaitable<void>>;
      {
        t.async_read(read_frame, close_code)
      } -> std::same_as<io::awaitable<void>>;
    };
} // namespace webpubsub