#pragma once
#include <WebPubSub/Protocols/Acks/SequenceAckSignal.hpp>
#include <WebPubSub/Protocols/Common/Constants.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/Exceptions/WebPubSubException.hpp>
#include <WebPubSub/Protocols/Requests/JoinGroupRequest.hpp>
#include <WebPubSub/Protocols/Requests/LeaveGroupRequest.hpp>
#include <WebPubSub/Protocols/Requests/SendToGroupRequest.hpp>
#include <WebPubSub/Protocols/Responses/AckResponse.hpp>
#include <WebPubSub/Protocols/Responses/ConnectedResponse.hpp>
#include <WebPubSub/Protocols/Responses/DisconnectedResponse.hpp>
#include <WebPubSub/Protocols/Responses/GroupMessageResponse.hpp>
#include <WebPubSub/Protocols/Responses/Response.hpp>
#include <WebPubSub/Protocols/Responses/ServerMessageResponse.hpp>
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>

namespace webpubsub {
// TODO: impl interface

class json_protocol_base {
public:
  std::string get_name() const { return "json.webpubsub.azure.v1"; }

  WebPubSubProtocolMessageType get_webpubsub_protocol_message_type() const {
    return WebPubSubProtocolMessageText;
  }

  bool is_reliable() const { return true; }

  template <request_t T> std::string write(const T &request) const {
    nlohmann::json json = request;
    return json.dump();
  }

  std::optional<ResponseVariant> read(const std::string &frame) const {
    namespace k = webpubsub::Constants::Keys;
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

static_assert(webpubsub_protocol_t<json_protocol_base>,
              "json_protocol_base doesn't implement webpubsub_protocol_t");
} // namespace webpubsub