#pragma once
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <WebPubSub/client/concepts/web_socket_t.hpp>
#include <optional>

namespace webpubsub {
template <typename T, typename WS>
concept web_socket_factory_t = requires(T t) {
  { t.create("uri", "protocol_name") } -> std::same_as<std::unique_ptr<WS>>;
} && requires(WS ws) {
  { ws } -> web_socket_t;
};
} // namespace webpubsub