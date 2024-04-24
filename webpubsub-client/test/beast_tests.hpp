#pragma once
#include "spdlog/spdlog.h"
#include "webpubsub/client/common/asio.hpp"
#include "webpubsub/client/config/core.hpp"
#include "gtest/gtest.h"

namespace test {
namespace beast {
TEST(test, beast_http) {
  namespace io = webpubsub::io;
  namespace beast = boost::beast;   // from <boost/beast.hpp>
  namespace http = beast::http;     // from <boost/beast/http.hpp>
  namespace net = boost::asio;      // from <boost/asio.hpp>
  using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>
  io::io_context ioc;
  io::co_spawn(
      ioc,
      []() -> io::awaitable<void> {
        auto host = "127.0.0.1";
        auto port = "5000";
        auto target = "/";
        int version = 11;
        // These objects perform our I/O
        // They use an executor with a default completion token of
        // use_awaitable This makes our code easy, but will use exceptions
        // as the default error handling, i.e. if the connection drops, we
        // might see an exception.
        auto resolver = net::use_awaitable.as_default_on(
            tcp::resolver(co_await net::this_coro::executor));
        auto stream = net::use_awaitable.as_default_on(
            beast::tcp_stream(co_await net::this_coro::executor));

        // Look up the domain name
        auto const results = co_await resolver.async_resolve(host, port);

        // Set the timeout.
        stream.expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        co_await stream.async_connect(results);

        // Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Set the timeout.
        stream.expires_after(std::chrono::seconds(30));

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

        // Gracefully close the socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        //
        if (ec && ec != beast::errc::not_connected)
          throw boost::system::system_error(ec, "shutdown");
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