#pragma once
#include <asio/awaitable.hpp>
#include <iostream>
#include <optional>
#include <webpubsub/client/async/task_cancellation/cancellation_token.hpp>
#include <webpubsub/client/common/web_socket/web_socket_close_status.hpp>
namespace webpubsub {
template <typename T>
concept web_socket_t =
    requires(T t, uint64_t *&read_ptr, uint64_t &read_size,
             web_socket_close_status &status, const uint64_t *write_ptr,
             const uint64_t write_size, const bool write_as_text,

             const std::optional<cancellation_token> &cancellation_token) {
      {
        t.async_connect(cancellation_token)
      } -> std::same_as<asio::awaitable<void>>;
      {
        t.async_write(write_ptr, write_size, write_as_text, cancellation_token)
      } -> std::same_as<asio::awaitable<void>>;
      {
        t.async_read(read_ptr, read_size, status, cancellation_token)
      } -> std::same_as<asio::awaitable<void>>;
    };
} // namespace webpubsub