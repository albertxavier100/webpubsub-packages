#pragma once
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/ReliableJsonV1Protocol.hpp>
#include <functional>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <vector>

struct ServerMessageValidator {
  void operator()(const WebPubSub::ServerMessageResponse &response) const {
    EXPECT_EQ(WebPubSub::Message, response.getType());
    EXPECT_EQ(678UL, response.getSequenceId());
    EXPECT_EQ(WebPubSub::Server, response.getFrom());
    EXPECT_EQ(WebPubSub::Text, response.getDataType());
    std::string data;
    response.getData(data);
    EXPECT_EQ("hello, world", data);
  }
  template <typename T, typename = std::enable_if_t<!std::is_same<
                            T, WebPubSub::ServerMessageResponse>::value>>
  void operator()(const T &response) const {}
};

TEST(ReadServerDataMessageResponse, Basic) {
  WebPubSub::ReliableJsonV1Protocol p;
  auto frame =
      R"({"sequenceId":678,"type":"message","from":"server","dataType":"text","data":"hello, world"})";
  auto res = p.read(frame);
  std::visit(ServerMessageValidator(), res.value());
}