#pragma once
// #include <Exceptions/WebPubSubException.hpp>
#include <WebPubSub/Protocols/Common/Common.hpp>
#include <WebPubSub/Protocols/Common/Constants.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <cppcodec/base64_rfc4648.hpp>
#include <optional>

namespace webpubsub {
template <typename TLatentAction, typename TWebSocket, typename TProtocol>
class RequestSender;

class Request {
  template <typename TLatentAction, typename TWebSocket, typename TProtocol>
  friend class RequestSender;

public:
  Request(const RequestType type, const std::optional<uint64_t> &ackId)
      : type(type), ackId(ackId) {}

protected:
  template <typename TRequest>
  static void requestToJson(nlohmann::json &json, const TRequest &request) {
    namespace k = webpubsub::Constants::Keys;
    json[k::type] = request.type;
    if (request.ackId.has_value()) {
      json[k::ackId] = request.ackId;
    }
  }

protected:
  const RequestType type;
  std::optional<uint64_t> ackId = std::nullopt;
};

class GroupRequest : public Request {
public:
  // TODO: validate group
  // TODO: limit type can be related to group
  GroupRequest(const RequestType type, const std::string &group,
               const std::optional<uint64_t> &ackId)
      : Request(type, ackId), group(group) {}

protected:
  template <typename TRequest>
  static void groupRequestToJson(nlohmann::json &json,
                                 const TRequest &request) {
    namespace k = webpubsub::Constants::Keys;
    requestToJson(json, request);
    json[k::group] = request.group;
  }

protected:
  const std::string &group;
};

class DataRequestHelper {
public:
  template <typename TData>
  static void dataToJson(nlohmann::json &json, const TData &inputData,
                         const std::optional<DataType> &dataType) {
    namespace k = webpubsub::Constants::Keys;
    using base64 = cppcodec::base64_rfc4648;

    switch (dataType.value_or(Json)) {
    case Text: {
      if (isString<TData>()) {
        json[k::data] = inputData;
      } else {
        nlohmann::json j = inputData;
        json[k::data] = j.dump();
      }
      break;
    }
    case Binary: {
      nlohmann::json j = inputData;
      std::string dumped = j.dump();
      std::string encoded = base64::encode(dumped);
      json[k::data] = encoded;
      break;
    }
    case Json:
    default: {
      if (std::is_class<TData>::value) {
        json[k::data] = inputData;
      } else {
        throw std::invalid_argument("not supported data to json");
      }
      break;
    }
    }
  }

private:
  template <typename T> static bool isString() {
    // todo: support more
    return std::is_same<T, std::string>::value ||
           std::is_same<T, const std::string>::value ||
           std::is_same<T, char[]>::value ||
           std::is_same<T, const char[]>::value ||
           std::is_same<T, char *>::value ||
           std::is_same<T, const char *>::value ||
           std::is_same<typename std::remove_cv<T>::type, char *>::value ||
           std::is_same<typename std::remove_cv<T>::type, const char *>::value;
  }

  template <typename T> static bool isBytes() {
    return std::is_same<T, unsigned char>::value ||
           std::is_same<T, signed char>::value ||
           std::is_same<T, char>::value ||
           std::is_same<T, unsigned char[]>::value ||
           std::is_same<T, signed char[]>::value ||
           std::is_same<T, char[]>::value ||
           std::is_same<T, const unsigned char[]>::value ||
           std::is_same<T, const signed char[]>::value ||
           std::is_same<T, const char[]>::value || std::is_array<T>::value ||
           std::is_same<T, std::vector<unsigned char>>::value ||
           std::is_same<T, std::vector<signed char>>::value ||
           std::is_same<T, std::vector<char>>::value;
  }
};
} // namespace webpubsub