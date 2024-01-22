#pragma once
#include <WebPubSub/Protocols/Acks/SequenceAckSignal.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/reliable_json_v1_protocol.hpp>
#include <WebPubSub/Protocols/Requests/JoinGroupRequest.hpp>
#include <WebPubSub/Protocols/Responses/AckResponse.hpp>
#include <functional>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
struct AckValidator {
  void operator()(const webpubsub::AckResponse &response) const {
    EXPECT_EQ(webpubsub::Ack, response.getType());
    EXPECT_EQ(789UL, response.getAckId());
    EXPECT_EQ("Forbidden", response.getError().value().getName());
    EXPECT_EQ("The reason", response.getError().value().getMessage());
    EXPECT_EQ(true, response.getSuccess());
  }
  template <typename T, typename = std::enable_if_t<
                            !std::is_same<T, webpubsub::AckResponse>::value>>
  void operator()(const T &response) const {}
};

TEST(ReadAckResponse, Basic) {
  webpubsub::reliable_json_v1_protocol p;
  auto frame =
      R"({"type":"ack","ackId":789,"success":true,"error":{"name":"Forbidden","message":"The reason"}})";
  auto res = p.read(frame);
  std::visit(AckValidator(), res.value());
}