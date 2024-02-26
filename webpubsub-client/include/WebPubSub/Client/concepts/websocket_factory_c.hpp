#pragma once
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <optional>
#include <webpubsub/client/concepts/websocket_t.hpp>

namespace webpubsub {
template <typename T, typename WS>
concept websocket_factory_t = requires(T t) {
  { t.create("uri", "protocol_name") } -> std::same_as<std::unique_ptr<WS>>;
} && requires(WS ws) {
  { ws } -> websocket_t;
};
} // namespace webpubsub