#pragma once
#include <WebPubSub/Protocols/Common/Common.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/Responses/Response.hpp>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace webpubsub {
class MessageResponseV2 : public Response {
public:
  MessageResponseV2() = default;
#pragma region Getter
  const std::optional<uint64_t> getSequenceId() const { return sequenceId; }
  const MessageSourceType getFrom() const { return from; }
  const std::optional<DataType> &getDataType() const { return dataType; }
#pragma region getData
  template <typename T> void getData(T &dataOut) const {
    if (dataType.value_or(Json) != Json ||
        data.type() != nlohmann::json::value_t::object) {
      throw std::invalid_argument(
          "DataType is not Json or json value is not object");
    }
    data.get_to(dataOut);
  }

  template <> void getData(std::vector<uint8_t> &dataOut) const {
    if (data.type() != nlohmann::json::value_t::string) {
      throw std::invalid_argument(
          "try to get binary data but json value is not string");
    }
    if (dataType.value_or(Json) != Binary) {
      throw std::invalid_argument(
          "try to get binary data but data type is not binary");
    }
    auto encoded = data.get<std::string>();
    using base64 = cppcodec::base64_rfc4648;
    dataOut = base64::decode(encoded);
    return;
  }
  template <> void getData(std::string &dataOut) const {
    switch (dataType.value_or(Json)) {
    case Text: {
      if (data.type() != nlohmann::json::value_t::string) {
        throw std::invalid_argument(
            "try to get text data but json value is not string");
      }
      data.get_to(dataOut);
      return;
    }
    case Json:
    default: {
      dataOut = data.dump();
      return;
    }
    }
  }
#pragma endregion
#pragma endregion

protected:
  std::optional<uint64_t> sequenceId = 0;
  MessageSourceType from = Group;
  std::optional<DataType> dataType = std::nullopt;
  nlohmann::json data = "{}"_json;
};
} // namespace webpubsub