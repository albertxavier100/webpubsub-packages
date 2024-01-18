#pragma once
#include <WebPubSub/Protocols/ReliableJsonV1Protocol.hpp>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

class TestData {
  int a = -1;
  int b = -1;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(TestData, a, b)

public:
  TestData() = default;
  TestData(int a, int b) : a(a), b(b) {}
};
template <typename T>
std::string writeSendToGroupRequest(const T &data,
                                    const std::optional<uint64_t> &ackId) {
  webpubsub::ReliableJsonV1Protocol p;
  std::string group = "mygroup";
  webpubsub::SendToGroupRequest<T> req(group, data, ackId, std::nullopt,
                                       std::nullopt);
  return p.write(req);
}

TEST(WriteSendToGroupRequest, Text_WithAckId) {
  std::string data(R"({"a":1,"b":2})");
  std::optional<uint64_t> ackId(123);

  EXPECT_EQ(
      std::string(
          "{\"ackId\":123,\"data\":\"{\\\"a\\\":1,\\\"b\\\":2}\",\"group\":"
          "\"mygroup\",\"type\":\"sendToGroup\"}"),
      writeSendToGroupRequest(data, ackId));
}

TEST(WriteSendToGroupRequest, Text_WithoutAckId) {
  std::string data(R"({"a":1,"b":2})");
  EXPECT_EQ(std::string("{\"data\":\"{\\\"a\\\":1,\\\"b\\\":2}\",\"group\":"
                        "\"mygroup\",\"type\":\"sendToGroup\"}"),
            writeSendToGroupRequest(data, std::nullopt));
}

TEST(WriteSendToGroupRequest, Json_WithoutAckId) {
  TestData data(3, 4);
  EXPECT_EQ(std::string("{\"data\":{\"a\":3,\"b\":4},\"group\":"
                        "\"mygroup\",\"type\":\"sendToGroup\"}"),
            writeSendToGroupRequest(data, std::nullopt));
}

TEST(WriteSendToGroupRequest, Binary_WithoutAckId) {
  std::vector<uint8_t> data = {65, 66, 67};
  EXPECT_EQ(std::string("{\"data\":[65,66,67],\"group\":"
                        "\"mygroup\",\"type\":\"sendToGroup\"}"),
            writeSendToGroupRequest(data, std::nullopt));
}