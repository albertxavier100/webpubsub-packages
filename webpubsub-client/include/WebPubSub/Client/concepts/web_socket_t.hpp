#pragma once
#include <asio/awaitable.hpp>
#include <iostream>
#include <optional>
#include <webpubsub/client/common/web_socket/web_socket_close_status.hpp>
namespace webpubsub {

// TODO: impl this, so that uint* and char* don't need to cast
template <typename T>
concept web_socket_t =
    requires(T t, std::string write_frame, std::string &read_frame,
             web_socket_close_status &status) {
      { t.async_connect() } -> std::same_as<asio::awaitable<void>>;
      { t.async_close() } -> std::same_as<asio::awaitable<void>>;
      { t.async_write(write_frame) } -> std::same_as<asio::awaitable<void>>;
      {
        t.async_read(read_frame, status)
      } -> std::same_as<asio::awaitable<void>>;
    };
} // namespace webpubsub