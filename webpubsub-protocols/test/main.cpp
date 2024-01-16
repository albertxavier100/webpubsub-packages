// request
// #include "ReliableJsonV1ProtocolTests_BasicRequest.hpp"
#include "ReliableJsonV1ProtocolTests_JoinGroupRequest.hpp"
#include "ReliableJsonV1ProtocolTests_LeaveGroupRequest.hpp"
#include "ReliableJsonV1ProtocolTests_SendEventRequest.hpp"
#include "ReliableJsonV1ProtocolTests_SendToGroupRequest.hpp"
#include "ReliableJsonV1ProtocolTests_SequenceAck.hpp"

// response
#include "ReliableJsonV1ProtocolTests_AckResponse.hpp"
#include "ReliableJsonV1ProtocolTests_ConnectedResponse.hpp"
#include "ReliableJsonV1ProtocolTests_DisconnectedResponse.hpp"
#include "ReliableJsonV1ProtocolTests_GroupDataMessageResponseV2.hpp"
#include "ReliableJsonV1ProtocolTests_ServerDataMessageResponse.hpp"

int main(int argc, char **argv) {
  // Initialize Google Test
  ::testing::InitGoogleTest(&argc, argv);

  // Run tests
  RUN_ALL_TESTS();
  return 0;
}