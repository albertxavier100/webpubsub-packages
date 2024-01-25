#pragma once
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>
#include <optional>

namespace webpubsub {
class json_v1_protocol {
public:
  json_v1_protocol() = default;
  std::optional<ResponseVariant> read(const char *frame) const {
    return baseProtocol.read(frame);
  }
  template <request_t T> std::string write(const T &request) const {
    return baseProtocol.write(request);
  }

  std::string get_name() const { return "json.webpubsub.azure.v1"; }

  bool is_reliable() const { return false; }

  WebPubSubProtocolMessageType get_webpubsub_protocol_message_type() const {
    return base_protocol_.get_webpubsub_protocol_message_type();
  }

private:
  json_protocol_base base_protocol_;
};
static_assert(webpubsub_protocol_t<json_v1_protocol>,
              "Concept webpubsub_protocol_t<json_v1_protocol> failed.");
} // namespace webpubsub
