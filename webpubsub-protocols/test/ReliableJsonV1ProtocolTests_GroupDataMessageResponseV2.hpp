#pragma once
#include <WebPubSub/Protocols/Acks/SequenceAckSignal.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/ReliableJsonV1Protocol.hpp>
#include <WebPubSub/Protocols/Requests/JoinGroupRequest.hpp>
#include <WebPubSub/Protocols/Responses/DisconnectedResponse.hpp>
#include <functional>
#include <gtest/gtest.h>
#include <variant>
#include <stdexcept>
#include <string>
#include <vector>

class Data {
  int a;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Data, a)
public:
  const int getA() const { return a; }
};

struct GroupMessageValidator {
  void operator()(const WebPubSub::GroupMessageResponseV2 &response) const {
    EXPECT_EQ(WebPubSub::Message, response.getType());
    EXPECT_EQ(1UL, response.getSequenceId());
    EXPECT_EQ(WebPubSub::Group, response.getFrom());
    EXPECT_EQ("group_name", response.getGroup());
    EXPECT_EQ(WebPubSub::Json, response.getDataType());
    Data data;
    response.getData(data);
    EXPECT_EQ(123, data.getA());
    EXPECT_EQ("abc", response.getFromUserId());
  }
  template <typename T, typename = std::enable_if_t<!std::is_same<
                            T, WebPubSub::GroupMessageResponseV2>::value>>
  void operator()(const T &response) const {}
};

TEST(ReadGroupDataMessageResponse, Basic) {
  WebPubSub::ReliableJsonV1Protocol p;
  auto frame =
      R"({"sequenceId":1,"type":"message","from":"group","group":"group_name","dataType":"json","data":{"a":123},"fromUserId":"abc"})";
  auto res = p.read(frame);
  std::visit(GroupMessageValidator(), res.value());
}