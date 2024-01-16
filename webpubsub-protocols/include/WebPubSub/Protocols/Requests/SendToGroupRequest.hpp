#pragma once
#include <WebPubSub/Protocols/Common/Common.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/Requests/Request.hpp>
#include <cppcodec/base64_rfc4648.hpp>

namespace WebPubSub {
template <typename TData> class SendToGroupRequest : public GroupRequest {
public:
  SendToGroupRequest(
      const std::string &group, const TData &data,
      const std::optional<uint64_t> &ackId = std::nullopt,
      const std::optional<bool> &noEcho = std::nullopt,
      const std::optional<DataType> &dataType = std::nullopt)
      : GroupRequest(SendToGroup, group, ackId), inputData(data),
        noEcho(noEcho), dataType(dataType) {}

private:
  template <typename UData = TData>
  friend void to_json(nlohmann::json &json,
                      const SendToGroupRequest<TData> &request) {
    namespace k = WebPubSub::Constants::Keys;
    groupRequestToJson(json, request);
    if (request.noEcho.has_value()) {
      json[k::noEcho] = request.noEcho;
    }
    DataRequestHelper::dataToJson(json, request.inputData, request.dataType);
  }

private:
  const std::optional<bool> &noEcho;
  const std::optional<DataType> &dataType;
  const TData inputData;
};
} // namespace WebPubSub