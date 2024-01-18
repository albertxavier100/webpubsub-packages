#pragma once
#include <WebPubSub/Protocols/Json/CustomSerializers.hpp>
#include <WebPubSub/Protocols/Responses/Response.hpp>

namespace webpubsub {
class ConnectedResponse : public Response {
public:
#pragma region Getter
  const SystemEventType getEvent() const { return event; }
  const std::optional<std::string> &getUserId() const { return userId; }
  const std::string &getConnectionId() const { return connectionId; }
  const std::optional<std::string> &getReconnectionToken() const {
    return reconnectionToken;
  }
#pragma endregion

private:
  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ConnectedResponse, type, event,
                                              connectionId, userId,
                                              reconnectionToken)

private:
  SystemEventType event = UnknownSystemEventType;
  std::optional<std::string> userId = std::nullopt;
  std::string connectionId = EMPTY_STRING;
  std::optional<std::string> reconnectionToken = std::nullopt;
};
} // namespace webpubsub