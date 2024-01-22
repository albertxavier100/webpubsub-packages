#pragma once
#include <WebPubSub/Protocols/reliable_json_v1_protocol.hpp>
#include <gtest/gtest.h>

std::string write_join_group_request(const std::optional<uint64_t> &ackId) {
  webpubsub::reliable_json_v1_protocol p;
  std::string group = "mygroup";
  webpubsub::JoinGroupRequest req(group, ackId);
  return p.write(req);
}

TEST(WriteJoinGroupRequest, WithAckId) {
  std::optional<uint64_t> ackId(123);
  auto request = write_join_group_request(ackId);
  EXPECT_EQ(
      std::string(R"({"ackId":123,"group":"mygroup","type":"joinGroup"})"),
      request);
}

TEST(WriteJoinGroupRequest, WithoutAckId) {
  auto request = write_join_group_request(std::nullopt);
  EXPECT_EQ(std::string(R"({"group":"mygroup","type":"joinGroup"})"), request);
}