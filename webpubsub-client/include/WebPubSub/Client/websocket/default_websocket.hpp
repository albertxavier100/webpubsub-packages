#pragma once
#include "spdlog/spdlog.h"
#include "uri.hh"
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/config/core.hpp"
#include <webpubsub/client/concepts/websocket_c.hpp>

namespace webpubsub {
class default_websocket {
  using websocket_t = io::beast::websocket::stream<
      io::beast::ssl_stream<io::beast::tcp_stream>>;

public:
  default_websocket(std::string uri, std::string protocol_name)
      : uri_(std::move(uri)), protocol_name_(std::move(protocol_name)) {}

  default_websocket &operator=(const default_websocket &other) {
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
    namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>

    uri client_access_uri(uri_);
    std::string host = client_access_uri.get_host();
    auto path = client_access_uri.get_path();
    auto query = client_access_uri.get_query();
    auto path_query = std::format("/{}?{}", path, query);
    auto port_num = client_access_uri.get_port();
    if (!port_num) {
      port_num = 443;
    }
    auto port = std::format("{}", port_num);
    spdlog::trace("host={0} port={1}, path={2}, query={3}", host, port, path,
                  query);

    ssl::context ctx{ssl::context::tlsv12_client};
    ctx.set_default_verify_paths();

    auto resolver = net::use_awaitable.as_default_on(
        tcp::resolver(co_await net::this_coro::executor));
    websocket_.emplace(websocket_t(co_await net::this_coro::executor, ctx));

    auto const results = co_await resolver.async_resolve(host, port);
    // TODO: make timeout as a member
    beast::get_lowest_layer(*websocket_)
        .expires_after(std::chrono::seconds(30));
    auto ep = co_await beast::get_lowest_layer(*websocket_)
                  .async_connect(results, io::use_awaitable);
    if (::SSL_set_tlsext_host_name(websocket_->next_layer().native_handle(),
                                   host.c_str())) {
      spdlog::trace("Failed to set SNI Hostname");
    }
    host += ':' + std::to_string(ep.port());
    websocket_->set_option(websocket::stream_base::decorator(
        [&protocol_name_ = protocol_name_](websocket::request_type &req) {
          req.set(http::field::user_agent, "webpubsub-client-cpp");
          req.set(http::field::sec_websocket_version, "13");
          req.set(http::field::sec_websocket_protocol, protocol_name_);
        }));
    beast::get_lowest_layer(*websocket_)
        .expires_after(std::chrono::seconds(30));
    websocket_->next_layer().handshake(ssl::stream_base::client);
    beast::get_lowest_layer(*websocket_).expires_never();
    websocket_->set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::client));
    co_await websocket_->async_handshake(host, path_query, io::use_awaitable);
  }

  auto async_close() -> io::awaitable<void> {
    co_await websocket_->async_close(io::beast::websocket::close_code::normal,
                                     io::use_awaitable);
  }

  auto async_write(std::string write_frame) -> io::awaitable<void> {
    co_await websocket_->async_write(
        io::buffer(std::string(std::move(write_frame))), io::use_awaitable);
  }

  auto async_read(std::string &read_frame, close_code_t &close_code)
      -> io::awaitable<void> {
    io::beast::flat_buffer buffer;
    co_await websocket_->async_read(buffer, io::use_awaitable);
    auto data = buffer.data();
    close_code = websocket_->reason().code;
    read_frame.assign(io::buffers_begin(data), io::buffers_end(data));
  }

private:
  std::string uri_;
  std::string protocol_name_;
  std::optional<websocket_t> websocket_;
};

static_assert(websocket_c<default_websocket>);
} // namespace webpubsub