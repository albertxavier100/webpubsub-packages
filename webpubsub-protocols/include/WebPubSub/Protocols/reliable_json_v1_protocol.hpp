#pragma once
#include <WebPubSub/Protocols/json_protocol_base.hpp>
#include <WebPubSub/Protocols/webpubsub_protocol_t.hpp>

namespace webpubsub {
class reliable_json_v1_protocol {
public:
  reliable_json_v1_protocol() = default;

  std::optional<ResponseVariant> read(const std::string frame) const {
    return base_protocol_.read(std::move(frame));
  }

  template <request_t T> std::string write(const T &request) const {
    return base_protocol_.write(request);
  }

  std::string get_name() const { return "json.reliable.webpubsub.azure.v1"; }

  WebPubSubProtocolMessageType get_webpubsub_protocol_message_type() const {
    return base_protocol_.get_webpubsub_protocol_message_type();
  }

  bool is_reliable() const { return true; }

private:
  json_protocol_base base_protocol_;
};

static_assert(
    webpubsub_protocol_t<reliable_json_v1_protocol>,
    "Concept webpubsub_protocol_t<reliable_json_v1_protocol> failed.");
} // namespace webpubsub