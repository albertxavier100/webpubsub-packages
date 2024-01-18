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
#include <optional>
#include <variant>

namespace webpubsub {
using ResponseVariant =
    std::variant<AckResponse, GroupMessageResponseV2, ServerMessageResponse,
                    ConnectedResponse, DisconnectedResponse>;

// template <typename TProtocol> struct IWebPubSubProtocolWrite {
//   template <typename UProtocol, typename TRequest,
//             typename = std::enable_if_t<
//                 std::is_base_of<Request, TRequest>::value &&
//                     !std::is_same<Request, TRequest>::value &&
//                     !std::is_same<GroupRequest, TRequest>::value ||
//                 std::is_same<SequenceAckSignal, TRequest>::value>>
//   static auto test(UProtocol *p)
//       -> decltype(p->write(std::declval<const TRequest &>()),
//       std::true_type());
//   template <typename U> static auto test(...) -> std::false_type;
//   static constexpr bool value = decltype(test<TProtocol>(nullptr))::value;
// };
//
// template <typename TProtocol> struct IWebPubSubProtocolRead {
//   template <typename UProtocol>
//   static auto test(UProtocol *p)
//       -> decltype(p->read(std::declval<const char *>()), std::true_type());
//   template <typename U> static auto test(...) -> std::false_type;
//   static constexpr bool value = decltype(test<TProtocol>(nullptr))::value;
// };
//
// #define WEBPUBSUB_P ROTOCOL_INTERFACE(T) \
//  std::enable_if_t<webpubsub::IWebPubSubProtocolWrite::value && \
//                   webpubsub::IWebPubSubProtocolRead::value>

template <typename Impl> struct IWebPubSubProtocol {
  std::optional<ResponseVariant> read(const char *frame) {
    return static_cast<Impl *>(this)->read(frame);
  }
  template <typename T, typename = std::enable_if_t<
                            std::is_base_of<Request, T>::value &&
                                !std::is_same<Request, T>::value &&
                                !std::is_same<GroupRequest, T>::value ||
                            std::is_same<SequenceAckSignal, T>::value>>
  std::string write(const T &request) {
    return static_cast<Impl *>(this)->write(request);
  }

  std::string getName() { return static_cast<Impl *>(this)->getName(); }

  WebPubSubProtocolMessageType getWebPubSubProtocolMessageType() {
    return static_cast<Impl *>(this)->getWebPubSubProtocolMessageType();
  }

  bool isReliable() { return static_cast<Impl *>(this)->isReliable(); }
};
} // namespace webpubsub