#pragma once
#include <WebPubSub/Protocols/Json/CustomSerializers.hpp>
#include <WebPubSub/Protocols/Responses/MessageResponse.hpp>

namespace WebPubSub {
class ServerMessageResponse : public MessageResponseV2 {
private:
  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ServerMessageResponse, sequenceId,
                                              type, from, dataType, data)
};
} // namespace WebPubSub