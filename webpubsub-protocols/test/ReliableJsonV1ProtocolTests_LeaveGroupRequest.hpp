#pragma once
#include <WebPubSub/Protocols/reliable_json_v1_protocol.hpp>
#include <gtest/gtest.h>

std::string writeLeaveGroupRequest(const std::optional<uint64_t> &ackId) {
  webpubsub::reliable_json_v1_protocol p;
  std::string group = "mygroup";
  webpubsub::LeaveGroupRequest req(group, ackId);
  return p.write(req);
}

TEST(WriteLeaveGroupRequest, WithAckId) {
  std::optional<uint64_t> ackId(123);
  auto request = writeLeaveGroupRequest(ackId);
  EXPECT_EQ(
      std::string(R"({"ackId":123,"group":"mygroup","type":"leaveGroup"})"),
      request);
}

TEST(WriteLeaveGroupRequest, WithoutAckId) {
  auto request = writeLeaveGroupRequest(std::nullopt);
  EXPECT_EQ(std::string(R"({"group":"mygroup","type":"leaveGroup"})"), request);
}