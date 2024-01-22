#pragma once
#include <asio/awaitable.hpp>
#include <iostream>
#include <optional>

namespace webpubsub {
template <typename T>
concept web_socket_t = requires(T t) {
  { t.async_connect() } -> std::same_as<asio::awaitable<void>>;
};
} // namespace webpubsub