#pragma once
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <optional>
#include <webpubsub/client/concepts/web_socket_t.hpp>

namespace webpubsub {
template <typename T, typename WS>
concept web_socket_factory_t = requires(T t) {
  { t.create("uri", "protocol_name") } -> std::same_as<std::unique_ptr<WS>>;
} && requires(WS ws) {
  { ws } -> web_socket_t;
};
} // namespace webpubsub