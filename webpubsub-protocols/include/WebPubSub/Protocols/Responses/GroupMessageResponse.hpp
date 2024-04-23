#pragma once
#include <WebPubSub/Protocols/Json/CustomSerializers.hpp>
#include <WebPubSub/Protocols/Responses/MessageResponse.hpp>

namespace webpubsub {
class GroupMessageResponseV2 : public MessageResponseV2 {

public:
#pragma region Getter
  const std::string &getGroup() const { return group; }
  const std::optional<std::string> &getFromUserId() const {
    return fromUserId;
  }
#pragma endregion
  auto moveGroup() -> std::string { return std::move(group); }
  auto moveFromUserId() -> std::optional<std::string> { return std::move(fromUserId); }

private:
  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(GroupMessageResponseV2,
                                              sequenceId, type, from, dataType,
                                              data, group, fromUserId)

private:
  std::string group = EMPTY_STRING;
  std::optional<std::string> fromUserId = std::nullopt;
};
} // namespace webpubsub