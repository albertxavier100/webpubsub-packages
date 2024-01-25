#pragma once
#include <../impl/default_web_socket.hpp>
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <optional>
#include <webpubsub/client/concepts/web_socket_factory_t.hpp>

namespace webpubsub {
// TODO: dev for now
class default_web_socket_factory {
public:
  default_web_socket_factory(asio::io_context &io_context)
      : io_context_(io_context) {}

  default_web_socket create(std::string url, std::string protocol_name) {
    // TODO: dev for now
    return default_web_socket(url, protocol_name, io_context_);
  }

private:
  asio::io_context &io_context_;
};

static_assert(web_socket_factory_t<default_web_socket_factory>);
} // namespace webpubsub