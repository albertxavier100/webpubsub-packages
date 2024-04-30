# webpubsub-packages

This is a header only library for web pubsub service.

## Requirements

- C++20 with co_await enabled.

## webpubsub-client

This's for send/receive messages around web pubsub service. Currently it supports Web PubSub JSON WebSocket subprotocol and Web PubSub Reliable JSON WebSocket subprotocol. By default, it uses [boost-beast](https://github.com/boostorg/beast) as websocket client. To integrate into your system, but you can also use your own websocket client, only if the custom websocket meet the [concept](./webpubsub-client/include/webpubsub/client/concepts/websocket_c.hpp). 

## webpubsub-protocol

This's for serializing and serializing webpubsub protocol. Currently it supports Web PubSub JSON WebSocket subprotocol and Web PubSub Reliable JSON WebSocket subprotocol.

## Usages

- [protocol basic sample](./webpubsub-protocols/test/main.cpp).
- [client basic sample](./webpubsub-client/test/client_with_beast_test.hpp).

## TODO

- Make protocol and client packages consistent in naming.
- Improve test coverage.