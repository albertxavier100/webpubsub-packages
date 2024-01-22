#pragma once
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/reliable_json_v1_protocol.hpp>
#include <WebPubSub/Protocols/Requests/JoinGroupRequest.hpp>
#include <WebPubSub/Protocols/Responses/ConnectedResponse.hpp>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <vector>

// TODO: add other tests
TEST(WriteGroupRequest, InvalidGroupName) {
  webpubsub::reliable_json_v1_protocol p;
  auto createEmptyGroupReq = []() {
    nonstd::optional<uint64_t> ackId;
    webpubsub::GroupRequest req(webpubsub::JoinGroup, "", ackId);
  };
  EXPECT_THROW(createEmptyGroupReq(), std::invalid_argument);
}

TEST(Dev, Only) {
  // std::string group("dadas");
  // nonstd::optional<uint64_t> ackId(123);
  // nonstd::optional<bool> noEcho;
  // nonstd::optional<webpubsub::DataType> dataType;
  // std::string content(R"({"a": 1, "b": 2})");
  // std::vector<uint8_t> d(content.begin(), content.end());
  // webpubsub::Data data(d);

  // webpubsub::SendToGroupRequest req(group, nonstd::nullopt, nonstd::nullopt,
  //                                   data, nonstd::nullopt);
}
