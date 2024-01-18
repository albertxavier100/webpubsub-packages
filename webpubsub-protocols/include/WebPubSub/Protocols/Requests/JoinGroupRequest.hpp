#pragma once
#include <WebPubSub/Protocols/Common/Common.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/Requests/Request.hpp>
#include <string>

namespace webpubsub {
class JoinGroupRequest : public GroupRequest {
public:
  JoinGroupRequest(const std::string &group,
                   const std::optional<uint64_t> &ackId = std::nullopt)
      : GroupRequest(JoinGroup, group, ackId) {}

private:
  friend void to_json(nlohmann::json &json, const JoinGroupRequest &request) {
    groupRequestToJson<JoinGroupRequest>(json, request);
  }
};
} // namespace webpubsub