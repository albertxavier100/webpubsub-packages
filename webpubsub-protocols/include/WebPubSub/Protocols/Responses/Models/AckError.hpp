#pragma once
#include <WebPubSub/Protocols/Common/Common.hpp>
#include <nlohmann/json.hpp>

namespace webpubsub {
class AckError {
public:
  inline const std::string &getName() const { return name; }
  inline const std::string &getMessage() const { return message; }

private:
  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AckError, name, message)

private:
  std::string name = EMPTY_STRING;
  std::string message = EMPTY_STRING;
};
} // namespace webpubsub