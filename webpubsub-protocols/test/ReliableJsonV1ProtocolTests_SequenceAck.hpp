#pragma once
#include <WebPubSub/Protocols/Acks/SequenceAckSignal.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/ReliableJsonV1Protocol.hpp>
#include <WebPubSub/Protocols/Requests/JoinGroupRequest.hpp>
#include <WebPubSub/Protocols/Responses/ConnectedResponse.hpp>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <vector>

// TODO: add other tests
TEST(WriteSequenceAck, Basic) {
  webpubsub::ReliableJsonV1Protocol p;
  uint64_t sequenceId = 456;
  webpubsub::SequenceAckSignal ack(sequenceId);
  EXPECT_EQ(std::string("{\"sequenceId\":456,\"type\":\"sequenceAck\"}"),
            p.write(ack));
}