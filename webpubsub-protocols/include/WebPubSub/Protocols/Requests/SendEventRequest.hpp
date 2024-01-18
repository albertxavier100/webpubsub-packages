#pragma once
#include <WebPubSub/Protocols/Common/Common.hpp>
#include <WebPubSub/Protocols/Requests/Request.hpp>

namespace webpubsub {
template <typename TData> class SendEventRequest : public Request {
public:
  SendEventRequest() = default;
  SendEventRequest(const std::string &event, const TData &data,
                   const std::optional<uint64_t> &ackId = std::nullopt,
                   const std::optional<DataType> &dataType = std::nullopt)
      : event(event), inputData(data), dataType(dataType),
        Request(SendCustomEvent, ackId) {}

private:
  template <typename UData = TData>
  friend void to_json(nlohmann::json &json,
                      const SendEventRequest<TData> &request) {
    namespace k = webpubsub::Constants::Keys;
    requestToJson(json, request);
    json[k::event] = request.event;
    DataRequestHelper::dataToJson(json, request.inputData, request.dataType);
  }

private:
  const std::string &event;
  const std::optional<DataType> &dataType;
  const TData inputData;
};
} // namespace webpubsub