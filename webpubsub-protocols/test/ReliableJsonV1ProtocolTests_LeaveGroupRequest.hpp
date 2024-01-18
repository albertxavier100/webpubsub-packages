#pragma once
#include <WebPubSub/Protocols/ReliableJsonV1Protocol.hpp>
#include <gtest/gtest.h>

std::string writeLeaveGroupRequest(const std::optional<uint64_t> &ackId) {
  webpubsub::ReliableJsonV1Protocol p;
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