#pragma once
#include <WebPubSub/Client/concepts/web_socket_factory_t.hpp>
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>

namespace webpubsub {
// TODO: dev for now
template <webpubsub_protocol_t WebPubSubProtocol = DummyWebPubSubProtocol>
class default_web_socket_factory {
public:
  default_web_socket create(std::string url, WebPubSubProtocol protocol) {
    // TODO: dev for now
    asio::io_context io_context;
    return default_web_socket(io_context);
  }
};

static_assert(
    web_socket_factory_t<default_web_socket_factory<DummyWebPubSubProtocol>>,
    "default_web_socket_factory doesn't implement web_socket_factory_t");
} // namespace webpubsub