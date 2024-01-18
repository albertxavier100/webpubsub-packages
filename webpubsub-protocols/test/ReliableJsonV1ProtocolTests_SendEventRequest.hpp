#pragma once
#include <WebPubSub/Protocols/ReliableJsonV1Protocol.hpp>
#include <WebPubSub/Protocols/Requests/SendEventRequest.hpp>
#include <gtest/gtest.h>

std::string writeSendEventRequest(const std::optional<uint64_t> &ackId) {
  webpubsub::ReliableJsonV1Protocol p;
  std::string event = "myevent";
  std::string data(R"({"a": 1, "b": 2})");
  webpubsub::SendEventRequest<std::string> req(event, data, ackId);
  return p.write(req);
}

TEST(WriteSendEventRequest, WithAckId) {
  std::optional<uint64_t> ackId(123);
  EXPECT_EQ(
      std::string("{\"ackId\":123,\"data\":\"{\\\"a\\\": 1, \\\"b\\\": 2}\","
                  "\"event\":\"myevent\","
                  "\"type\":\"event\"}"),
      writeSendEventRequest(ackId));
}

TEST(WriteSendEventRequest, WithoutAckId) {
  EXPECT_EQ(
      std::string(
          "{\"data\":\"{\\\"a\\\": 1, \\\"b\\\": 2}\",\"event\":\"myevent\","
          "\"type\":\"event\"}"),
      writeSendEventRequest(std::nullopt));
}