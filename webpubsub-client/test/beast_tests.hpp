#pragma once
#include "spdlog/spdlog.h"
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/config/core.hpp"
#include "gtest/gtest.h"
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

namespace test {
namespace beast {
TEST(test, beast_http) {
  namespace io = webpubsub::io;
  namespace beast = boost::beast;   // from <boost/beast.hpp>
  namespace http = beast::http;     // from <boost/beast/http.hpp>
  namespace net = boost::asio;      // from <boost/asio.hpp>
  using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>
  namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>

  io::io_context ioc;
  
  io::co_spawn(
      ioc,
      []() -> io::awaitable<void> {
        std::string host = "127.0.0.1";
        auto port = "5000";
        auto target = "/";
        int version = 11;

        ssl::context ctx{ssl::context::tlsv12_client};

        // These objects perform our I/O
        // They use an executor with a default completion token of use_awaitable
        // This makes our code easy, but will use exceptions as the default
        // error handling, i.e. if the connection drops, we might see an
        // exception. See async_shutdown for error handling with an error_code.
        auto resolver = net::use_awaitable.as_default_on(
            tcp::resolver(co_await net::this_coro::executor));
        using executor_with_default =
            net::use_awaitable_t<>::executor_with_default<net::any_io_executor>;
        using tcp_stream = typename beast::tcp_stream::rebind_executor<
            executor_with_default>::other;

        // We construct the ssl stream from the already rebound tcp_stream.
        beast::ssl_stream<tcp_stream> stream{
            net::use_awaitable.as_default_on(
                beast::tcp_stream(co_await net::this_coro::executor)),
            ctx};

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
          throw boost::system::system_error(static_cast<int>(::ERR_get_error()),
                                            net::error::get_ssl_category());

        // Look up the domain name
        auto const results = co_await resolver.async_resolve(host, port);

        // Set the timeout.
        beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        co_await beast::get_lowest_layer(stream).async_connect(results);

        // Set the timeout.
        beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

        // Perform the SSL handshake
        co_await stream.async_handshake(ssl::stream_base::client);

        // Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Set the timeout.
        beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

        // Send the HTTP request to the remote host
        co_await http::async_write(stream, req);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer b;

        // Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        co_await http::async_read(stream, b, res);

        // Write the message to standard out
        std::cout << res << std::endl;

        // Set the timeout.
        beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

        // Gracefully close the stream - do not threat every error as an
        // exception!
        auto [ec] =
            co_await stream.async_shutdown(net::as_tuple(net::use_awaitable));
        if (ec == net::error::eof) {
          // Rationale:
          // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
          ec = {};
        }
        if (ec)
          throw boost::system::system_error(ec, "shutdown");

        // If we get here then the connection is closed gracefully

      },
      io::detached);
  ioc.run();
}

TEST(test, beast_websocket) {

  namespace beast = boost::beast;         // from <boost/beast.hpp>
  namespace http = beast::http;           // from <boost/beast/http.hpp>
  namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
  namespace net = boost::asio;            // from <boost/asio.hpp>
  using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
  namespace io = webpubsub::io;
  namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>

  std::string host = "127.0.0.1";
  auto port = "8002";
  auto text = "hello";

  io::io_context ioc;
  io::co_spawn(
      ioc,
      [&host, &port, &text]() -> io::awaitable<void> {
        // These objects perform our I/O
        auto resolver = net::use_awaitable.as_default_on(
            tcp::resolver(co_await net::this_coro::executor));
        auto ws = net::use_awaitable.as_default_on(
            websocket::stream<beast::tcp_stream>(
                co_await net::this_coro::executor));

        // Look up the domain name
        auto const results = co_await resolver.async_resolve(host, port);

        // Set a timeout on the operation
        beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        auto ep = co_await beast::get_lowest_layer(ws).async_connect(results);

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        host += ':' + std::to_string(ep.port());

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        beast::get_lowest_layer(ws).expires_never();

        // Set suggested timeout settings for the websocket
        ws.set_option(websocket::stream_base::timeout::suggested(
            beast::role_type::client));

        // Set a decorator to change the User-Agent of the handshake
        ws.set_option(
            websocket::stream_base::decorator([](websocket::request_type &req) {
              req.set(http::field::user_agent,
                      std::string(BOOST_BEAST_VERSION_STRING) +
                          " websocket-client-coro");
            }));

        // Perform the websocket handshake
        co_await ws.async_handshake(host, "/");

        // Send the message
        co_await ws.async_write(net::buffer(std::string(text)));

        // This buffer will hold the incoming message
        beast::flat_buffer buffer;

        // Read a message into our buffer
        co_await ws.async_read(buffer);

        // Close the WebSocket connection
        co_await ws.async_close(websocket::close_code::normal);

        // If we get here then the connection is closed gracefully

        // The make_printable() function helps print a ConstBufferSequence
        std::cout << beast::make_printable(buffer.data()) << std::endl;
      },
      io::detached);
  ioc.run();
}
} // namespace beast
} // namespace test