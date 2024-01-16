#pragma once
#include <WebPubSub/Protocols/Acks/SequenceAckSignal.hpp>
#include <WebPubSub/Protocols/Common/Constants.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/Exceptions/WebPubSubException.hpp>
#include <WebPubSub/Protocols/IWebPubSubProtocol.hpp>
#include <WebPubSub/Protocols/Requests/JoinGroupRequest.hpp>
#include <WebPubSub/Protocols/Requests/LeaveGroupRequest.hpp>
#include <WebPubSub/Protocols/Requests/SendToGroupRequest.hpp>
#include <WebPubSub/Protocols/Responses/AckResponse.hpp>
#include <WebPubSub/Protocols/Responses/ConnectedResponse.hpp>
#include <WebPubSub/Protocols/Responses/DisconnectedResponse.hpp>
#include <WebPubSub/Protocols/Responses/GroupMessageResponse.hpp>
#include <WebPubSub/Protocols/Responses/Response.hpp>
#include <WebPubSub/Protocols/Responses/ServerMessageResponse.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>

namespace WebPubSub {
// TODO: impl interface

class JsonProtocolBase : public IWebPubSubProtocol<JsonProtocolBase> {
public:
  std::string getName() { return "json.webpubsub.azure.v1"; }

  WebPubSubProtocolMessageType getWebPubSubProtocolMessageType() {
    return WebPubSubProtocolMessageText;
  }

  bool isReliable() { true; }

  template <typename T, typename = std::enable_if_t<
                            std::is_base_of<Request, T>::value &&
                                !std::is_same<Request, T>::value &&
                                !std::is_same<GroupRequest, T>::value ||
                            std::is_same<SequenceAckSignal, T>::value>>
  std::string write(const T &request) {
    nlohmann::json json = request;
    return json.dump();
  }

  // TODO: only support webSocket text for now
  std::optional<ResponseVariant> read(const char *frame) {
    namespace k = WebPubSub::Constants::Keys;
    auto json = nlohmann::json::parse(frame);

    ResponseType type = json[k::type];
    switch (type) {
    case Ack: {
      return json.get<AckResponse>();
    }
    case Message: {
      MessageSourceType from = json[k::from];
      switch (from) {
      case Group: {
        return json.get<GroupMessageResponseV2>();
      }
      case Server:
      default: {
        return json.get<ServerMessageResponse>();
      }
      }
    }
    case System: {
      SystemEventType event = json[k::event];
      switch (event) {
      case Connected: {
        return json.get<ConnectedResponse>();
      }
      case Disconnected: {
        return json.get<DisconnectedResponse>();
      }
      default:
        return std::nullopt;
      }
    }
    default:
      return std::nullopt;
    };
  }
};
} // namespace WebPubSub