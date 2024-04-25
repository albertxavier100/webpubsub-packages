#pragma once
#include <../impl/default_web_socket.hpp>
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <optional>
#include <webpubsub/client/concepts/websocket_factory_c.hpp>

namespace webpubsub {
class default_websocket_factory {
public:
  default_websocket_factory() {}

  default_web_socket create(std::string url, std::string protocol_name) {
    return default_web_socket(url, protocol_name);
  }
};

static_assert(web_socket_factory_t<default_websocket_factory>);
} // namespace webpubsub