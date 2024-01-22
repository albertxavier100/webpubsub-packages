#pragma once
#include <WebPubSub/Client/concepts/web_socket_t.hpp>
#include <WebPubSub/Protocols/IWebPubSubProtocol.hpp>
namespace webpubsub {
template <typename T>
concept web_socket_factory_t = requires(T t) {
  { t.create("", DummyWebPubSubProtocol()) } -> web_socket_t;
};
} // namespace webpubsub