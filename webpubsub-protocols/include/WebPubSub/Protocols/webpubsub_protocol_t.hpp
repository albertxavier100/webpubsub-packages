#pragma once
#include <WebPubSub/Protocols/Acks/SequenceAckSignal.hpp>
#include <WebPubSub/Protocols/Requests/JoinGroupRequest.hpp>
#include <WebPubSub/Protocols/Requests/LeaveGroupRequest.hpp>
#include <WebPubSub/Protocols/Requests/SendToGroupRequest.hpp>
#include <WebPubSub/Protocols/Responses/AckResponse.hpp>
#include <WebPubSub/Protocols/Responses/ConnectedResponse.hpp>
#include <WebPubSub/Protocols/Responses/DisconnectedResponse.hpp>
#include <WebPubSub/Protocols/Responses/GroupMessageResponse.hpp>
#include <WebPubSub/Protocols/Responses/Response.hpp>
#include <WebPubSub/Protocols/Responses/ServerMessageResponse.hpp>
#include <concepts>
#include <optional>
#include <variant>

namespace webpubsub {
using ResponseVariant =
    std::variant<AckResponse, GroupMessageResponseV2, ServerMessageResponse,
                 ConnectedResponse, DisconnectedResponse>;

template <typename T>
concept request_t =
    std::is_base_of<Request, T>::value && !std::is_same<Request, T>::value &&
        !std::is_same<GroupRequest, T>::value ||
    std::is_same<SequenceAckSignal, T>::value;

// TODO: not finish
template <typename T>
concept webpubsub_protocol_t = requires(T t) {
  { t.read("") } -> std::same_as<std::optional<ResponseVariant>>;
  { t.write(JoinGroupRequest("")) } -> std::same_as<std::string>;
  { t.get_name() } -> std::same_as<std::string>;
  {
    t.get_webpubsub_protocol_message_type()
  } -> std::same_as<WebPubSubProtocolMessageType>;
  {t.is_reliable()}->std::same_as<bool>;
};

class DummyWebPubSubProtocol {
public:
  DummyWebPubSubProtocol() {}
  std::optional<ResponseVariant> read(const char *frame) const {
    return std::nullopt;
  }

  template <request_t T> std::string write(const T &request) const {
    return "";
  }

  std::string get_name() { return ""; }

  bool is_reliable() {return false;}
  WebPubSubProtocolMessageType get_webpubsub_protocol_message_type() const {
    return WebPubSubProtocolMessageText;
  }
};
static_assert(webpubsub_protocol_t<DummyWebPubSubProtocol>,
              "Check DummyWebPubSubProtocol");
} // namespace webpubsub