#pragma once
#include <asio/awaitable.hpp>
#include <iostream>
#include <optional>
#include <WebPubSub/Client/Models/RequestResult.hpp>
namespace webpubsub {
template <typename T>
concept IWebSocket = requires(T t) {
  { t.connectAsync() } -> std::same_as<asio::awaitable<void>>;
};
} // namespace webpubsub