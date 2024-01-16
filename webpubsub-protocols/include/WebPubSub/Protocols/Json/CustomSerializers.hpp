#pragma once
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <nlohmann/json.hpp>
#include <optional>

#pragma region Optional Serializer
namespace nlohmann {
template <typename T> struct adl_serializer<std::optional<T>> {
  static void to_json(json &j, const std::optional<T> &opt) {
    if (opt == std::nullopt) {
      j = nullptr;
    } else {
      j = opt.value();
    }
  }

  static void from_json(const json &j, std::optional<T> &opt) {
    if (j.is_null()) {
      opt = std::nullopt;
    } else {
      opt = j.template get<T>();
    }
  }
};
} // namespace nlohmann
#pragma endregion

#pragma region Enum Serializers
namespace WebPubSub {
NLOHMANN_JSON_SERIALIZE_ENUM(RequestType, {
                                              {JoinGroup, "joinGroup"},
                                              {LeaveGroup, "leaveGroup"},
                                              {SendToGroup, "sendToGroup"},
                                              {SendCustomEvent, "event"},
                                              {SequenceAck, "sequenceAck"},
                                          })
NLOHMANN_JSON_SERIALIZE_ENUM(ResponseType, {
                                               {Ack, "ack"},
                                               {Message, "message"},
                                               {System, "system"},
                                           })
NLOHMANN_JSON_SERIALIZE_ENUM(DataType, {
                                           {Json, "json"},
                                           {Text, "text"},
                                           {Binary, "binary"},
                                       })
NLOHMANN_JSON_SERIALIZE_ENUM(SystemEventType,
                             {
                                 {UnknownSystemEventType, nullptr},
                                 {Connected, "connected"},
                                 {Disconnected, "disconnected"},
                             })
NLOHMANN_JSON_SERIALIZE_ENUM(MessageSourceType,
                             {
                                 {UnknownMessageSourceType, nullptr},
                                 {Server, "server"},
                                 {Group, "group"},
                             })
} // namespace WebPubSub
#pragma endregion