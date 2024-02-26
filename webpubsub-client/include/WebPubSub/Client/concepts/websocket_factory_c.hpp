#pragma once
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <optional>
#include <webpubsub/client/concepts/websocket_c.hpp>

namespace webpubsub {
template <typename T, typename WS>
concept websocket_factory_c = requires(T t) {
  { t.create("uri", "protocol_name") } -> std::same_as<std::unique_ptr<WS>>;
} && requires(WS ws) {
  { ws } -> websocket_c;
};
} // namespace webpubsub