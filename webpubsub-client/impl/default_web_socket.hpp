#pragma once
#include "spdlog/spdlog.h"
#include "uri.hh"
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/common/websocket/websocket_close_status.hpp"
#include "webpubsub/client/config/core.hpp"
#include <webpubsub/client/concepts/websocket_c.hpp>

namespace webpubsub {
  // TODO: rename
class default_web_socket {
public:
  default_web_socket(std::string uri, std::string protocol_name)
      :  uri_(std::move(uri)),
        protocol_name_(std::move(protocol_name)) {}

  default_web_socket &operator=(const default_web_socket &other) {
    if (this != &other) {
      uri_ = other.uri_;
      protocol_name_ = other.protocol_name_;
    }

    return *this;
  }

  auto async_connect() -> io::awaitable<void> {
    namespace beast = io::beast;            // from <boost/beast.hpp>
    namespace http = beast::http;           // from <boost/beast/http.hpp>
    namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
    namespace net = io;                     // from <boost/asio.hpp>
    using tcp = io::ip::tcp;                // from <boost/asio/ip/tcp.hpp>

    uri client_access_uri(uri_);
    std::string host = client_access_uri.get_host();
    auto port = "443";

    auto resolver = net::use_awaitable.as_default_on(
        tcp::resolver(co_await net::this_coro::executor));
    auto ws =
        net::use_awaitable.as_default_on(websocket::stream<beast::tcp_stream>(
            co_await net::this_coro::executor));

    auto const results = co_await resolver.async_resolve(host, port);
    // TODO: make timeout as a member
    beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(30));
    auto ep = co_await beast::get_lowest_layer(ws).async_connect(results);
    host += ':' + std::to_string(ep.port());
    beast::get_lowest_layer(ws).expires_never();
    ws.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::client));
    ws.set_option(
        websocket::stream_base::decorator([](websocket::request_type &req) {
          req.set(http::field::user_agent,
                  std::string(BOOST_BEAST_VERSION_STRING) +
                      " websocket-client-coro");
        }));
    co_await ws.async_handshake(host, "/");
  }

  auto async_close() -> io::awaitable<void> { co_return; }

  io::awaitable<void> async_write(std::string write_frame) { co_return; }

  io::awaitable<void> async_read(std::string &read_frame,
                                 websocket_close_status &status) {
    co_return;
  }

private:

private:
  std::string uri_;
  std::string protocol_name_;
};

static_assert(websocket_c<default_web_socket>);
} // namespace webpubsub