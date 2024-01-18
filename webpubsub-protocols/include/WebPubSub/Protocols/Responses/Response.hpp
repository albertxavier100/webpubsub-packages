#pragma once
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <nlohmann/json.hpp>
namespace webpubsub {
class Response {
public:
  Response() = default;
#pragma region Getter
  const ResponseType getType() const { return type; }
#pragma endregion

protected:
  Response(ResponseType type) : type(type) {}

protected:
  ResponseType type = UnknownResponseType;
};
} // namespace webpubsub