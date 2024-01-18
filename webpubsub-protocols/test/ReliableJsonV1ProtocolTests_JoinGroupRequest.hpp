#pragma once
#include <WebPubSub/Protocols/ReliableJsonV1Protocol.hpp>
#include <gtest/gtest.h>

std::string writeJoinGroupRequest(const std::optional<uint64_t> &ackId) {
  webpubsub::ReliableJsonV1Protocol p;
  std::string group = "mygroup";
  webpubsub::JoinGroupRequest req(group, ackId);
  return p.write(req);
}

TEST(WriteJoinGroupRequest, WithAckId) {
  std::optional<uint64_t> ackId(123);
  auto request = writeJoinGroupRequest(ackId);
  EXPECT_EQ(
      std::string(R"({"ackId":123,"group":"mygroup","type":"joinGroup"})"),
      request);
}

TEST(WriteJoinGroupRequest, WithoutAckId) {
  auto request = writeJoinGroupRequest(std::nullopt);
  EXPECT_EQ(std::string(R"({"group":"mygroup","type":"joinGroup"})"), request);
}