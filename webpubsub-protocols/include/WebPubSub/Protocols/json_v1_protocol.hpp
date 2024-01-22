#pragma once
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <optional>

namespace webpubsub {
class json_v1_protocol {
public:
  json_v1_protocol() = default;
  std::optional<ResponseVariant> read(const char *frame) {
    return baseProtocol.read(frame);
  }
  template <request_t T> std::string write(const T &request) {
    return baseProtocol.write(request);
  }

  std::string get_name() { return "json.webpubsub.azure.v1"; }

  bool is_reliable() { return false; }

  WebPubSubProtocolMessageType get_webpubsub_protocol_message_type() {
    return base_protocol_.get_webpubsub_protocol_message_type();
  }

private:
  json_protocol_base base_protocol_;
};
static_assert(
    webpubsub_protocol_t<json_v1_protocol>,
    "Concept webpubsub_protocol_t<json_v1_protocol> failed.");
} // namespace webpubsub
