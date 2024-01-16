#pragma once

#include <gtest/gtest.h>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

using namespace std::chrono_literals;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Sends a WebSocket message and prints the response
net::awaitable<void> do_session(net::io_context &ioc, std::string host,
                                std::string port, std::string text) {
  // These objects perform our I/O
  auto resolver = net::use_awaitable.as_default_on(
      tcp::resolver(co_await net::this_coro::executor));
  auto ws = net::use_awaitable.as_default_on(
      websocket::stream<beast::tcp_stream>(co_await net::this_coro::executor));

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
  ws.set_option(
      websocket::stream_base::timeout::suggested(beast::role_type::client));

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

  // Read a message into our buffer
  for (;;) {
    beast::flat_buffer buffer;
    co_await ws.async_read(buffer);
    std::cout << beast::make_printable(buffer.data()) << std::endl;

    boost::asio::steady_timer timer(ioc, 1s);
    co_await timer.async_wait(boost::asio::use_awaitable);

    co_await ws.async_write(net::buffer(std::string(text)));
  }

  // Close the WebSocket connection
  co_await ws.async_close(websocket::close_code::normal);

  // If we get here then the connection is closed gracefully
}

//------------------------------------------------------------------------------

int mainxx(
    // int argc, char **argv
) {
  auto const host = "localhost";
  auto const port = "40510";
  auto const text = "hello";

  // The io_context is required for all I/O
  net::io_context ioc;

  // Launch the asynchronous operation
  net::co_spawn(ioc, do_session(ioc, host, port, text),
                [](std::exception_ptr e) {
                  if (e)
                    try {
                      std::rethrow_exception(e);
                    } catch (std::exception &e) {
                      std::cerr << "Error: " << e.what() << "\n";
                    }
                });

  // Run the I/O service. The call will return when
  // the socket is closed.
  ioc.run();

  return EXIT_SUCCESS;
}

TEST(Client, Basic) { mainxx(); }