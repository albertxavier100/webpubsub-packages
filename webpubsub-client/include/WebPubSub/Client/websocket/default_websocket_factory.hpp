#pragma once
#include "webpubsub/client/websocket/default_websocket.hpp"
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <optional>
#include <webpubsub/client/concepts/websocket_factory_c.hpp>

namespace webpubsub {
class default_websocket_factory {
public:
  default_websocket_factory() {}

  auto create(std::string url, std::string protocol_name) -> std::unique_ptr<default_websocket> {
    return std::move(std::make_unique<default_websocket>(url, protocol_name));
  }
};

static_assert(
    websocket_factory_c<default_websocket_factory, default_websocket>);
} // namespace webpubsub