#pragma once
#include <WebPubSub/Protocols/Json/CustomSerializers.hpp>
#include <WebPubSub/Protocols/Responses/Response.hpp>

namespace webpubsub {
class DisconnectedResponse : public Response {
public:
#pragma region Getter
  const SystemEventType getEvent() const { return event; }
  const std::optional<std::string> &getMessage() const { return message; }
#pragma endregion

private:
  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DisconnectedResponse, type, event,
                                              message)

private:
  SystemEventType event = UnknownSystemEventType;
  std::optional<std::string> message = std::nullopt;
};
} // namespace webpubsub