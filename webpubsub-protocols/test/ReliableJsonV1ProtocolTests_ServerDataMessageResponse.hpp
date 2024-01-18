#pragma once
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/ReliableJsonV1Protocol.hpp>
#include <functional>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <vector>

struct ServerMessageValidator {
  void operator()(const webpubsub::ServerMessageResponse &response) const {
    EXPECT_EQ(webpubsub::Message, response.getType());
    EXPECT_EQ(678UL, response.getSequenceId());
    EXPECT_EQ(webpubsub::Server, response.getFrom());
    EXPECT_EQ(webpubsub::Text, response.getDataType());
    std::string data;
    response.getData(data);
    EXPECT_EQ("hello, world", data);
  }
  template <typename T, typename = std::enable_if_t<!std::is_same<
                            T, webpubsub::ServerMessageResponse>::value>>
  void operator()(const T &response) const {}
};

TEST(ReadServerDataMessageResponse, Basic) {
  webpubsub::ReliableJsonV1Protocol p;
  auto frame =
      R"({"sequenceId":678,"type":"message","from":"server","dataType":"text","data":"hello, world"})";
  auto res = p.read(frame);
  std::visit(ServerMessageValidator(), res.value());
}