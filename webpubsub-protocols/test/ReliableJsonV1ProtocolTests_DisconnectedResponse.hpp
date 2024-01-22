#pragma once
#include <WebPubSub/Protocols/Acks/SequenceAckSignal.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/reliable_json_v1_protocol.hpp>
#include <WebPubSub/Protocols/Requests/JoinGroupRequest.hpp>
#include <WebPubSub/Protocols/Responses/DisconnectedResponse.hpp>
#include <functional>
#include <gtest/gtest.h>
#include <variant>
#include <stdexcept>
#include <string>
#include <vector>
struct DisconnectedValidator {
  void operator()(const webpubsub::DisconnectedResponse &response) const {
    EXPECT_EQ(webpubsub::System, response.getType());
    EXPECT_EQ(webpubsub::Disconnected, response.getEvent());
    EXPECT_EQ("reason", response.getMessage().value());
  }
  template <typename T, typename = std::enable_if_t<!std::is_same<
                            T, webpubsub::DisconnectedResponse>::value>>
  void operator()(const T &response) const {}
};

TEST(ReadDisconnectedResponse, Basic) {
  webpubsub::reliable_json_v1_protocol p;
  auto frame = R"({"type":"system","event":"disconnected","message":"reason"})";
  auto res = p.read(frame);
  std::visit(DisconnectedValidator(), res.value());
}