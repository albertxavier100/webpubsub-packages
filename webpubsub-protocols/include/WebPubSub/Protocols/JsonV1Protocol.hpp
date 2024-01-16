#pragma once
#include <WebPubSub/Protocols/IWebPubSubProtocol.hpp>
#include <WebPubSub/Protocols/JsonProtocolBase.hpp>

namespace WebPubSub {
class JsonV1Protocol : public IWebPubSubProtocol<JsonV1Protocol> {
public:
  JsonV1Protocol() = default;
  nonstd::optional<ResponseVariant> read(const char *frame) {
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

  std::string getName() { return "json.webpubsub.azure.v1"; }

  bool isReliable() { return false; }

private:
  JsonProtocolBase baseProtocol;
};
} // namespace WebPubSub
