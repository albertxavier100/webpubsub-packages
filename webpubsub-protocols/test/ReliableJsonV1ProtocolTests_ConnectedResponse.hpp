#pragma once
#include <WebPubSub/Protocols/Acks/SequenceAckSignal.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/reliable_json_v1_protocol.hpp>
#include <WebPubSub/Protocols/Requests/JoinGroupRequest.hpp>
#include <WebPubSub/Protocols/Responses/ConnectedResponse.hpp>
#include <functional>
#include <gtest/gtest.h>
#include <variant>
#include <stdexcept>
#include <string>
#include <vector>
struct ConnectedValidator {
  void operator()(const webpubsub::ConnectedResponse &response) const {
    EXPECT_EQ(webpubsub::System, response.getType());
    EXPECT_EQ(webpubsub::Connected, response.getEvent());
    EXPECT_EQ("abcdefghijklmnop", response.getConnectionId());
    EXPECT_EQ("<token>", response.getReconnectionToken().value());
  }
  template <typename T, typename = std::enable_if_t<!std::is_same<
                            T, webpubsub::ConnectedResponse>::value>>
  void operator()(const T &response) const {}
};

TEST(ReadConnectedResponse, Basic) {
  webpubsub::reliable_json_v1_protocol p;
  auto frame =
      R"({"type":"system","event":"connected","userId":"user1","connectionId":"abcdefghijklmnop","reconnectionToken":"<token>"})";
  auto res = p.read(frame);
  std::visit(ConnectedValidator(), res.value());
}