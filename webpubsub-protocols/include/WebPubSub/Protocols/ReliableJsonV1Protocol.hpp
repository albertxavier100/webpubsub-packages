#pragma once
#include <WebPubSub/Protocols/IWebPubSubProtocol.hpp>
#include <WebPubSub/Protocols/JsonProtocolBase.hpp>

namespace WebPubSub {
class ReliableJsonV1Protocol
    : public IWebPubSubProtocol<ReliableJsonV1Protocol> {
public:
  ReliableJsonV1Protocol() = default;
  std::optional<ResponseVariant> read(const char *frame) {
    return baseProtocol.read(frame);
  }
  template <typename T, typename = std::enable_if_t<
                            std::is_base_of<Request, T>::value &&
                                !std::is_same<Request, T>::value &&
                                !std::is_same<GroupRequest, T>::value ||
                            std::is_same<SequenceAckSignal, T>::value>>
  std::string write(const T &request) {
    return baseProtocol.write(request);
  }

  std::string getName() { return "json.reliable.webpubsub.azure.v1"; }

  bool isReliable() { return true; }

private:
  JsonProtocolBase baseProtocol;
};
} // namespace WebPubSub
