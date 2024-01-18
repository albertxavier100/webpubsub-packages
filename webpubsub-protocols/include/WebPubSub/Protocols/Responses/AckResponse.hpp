#pragma once
#include <WebPubSub/Protocols/Json/CustomSerializers.hpp>
#include <WebPubSub/Protocols/Responses/Models/AckError.hpp>
#include <WebPubSub/Protocols/Responses/Response.hpp>

namespace webpubsub {
class AckResponse : public Response {
public:
#pragma region Getter
  const uint64_t &getAckId() const { return ackId; }
  const bool getSuccess() const { return success; }
  const std::optional<AckError> &getError() const { return error; }
#pragma endregion

private:
  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AckResponse, type, ackId, error,
                                              success)
private:
  uint64_t ackId = -1;
  std::optional<AckError> error = std::nullopt;
  bool success = false;
};
} // namespace webpubsub