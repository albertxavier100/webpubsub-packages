#pragma once
#include <WebPubSub/Protocols/Common/Common.hpp>
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/Requests/Request.hpp>
#include <string>
namespace WebPubSub {
class LeaveGroupRequest : public GroupRequest {
public:
  LeaveGroupRequest(const std::string &group,
                    const std::optional<uint64_t> &ackId = std::nullopt)
      : GroupRequest(LeaveGroup, group, ackId) {}

private:
  friend void to_json(nlohmann::json &json, const LeaveGroupRequest &request) {
    groupRequestToJson<LeaveGroupRequest>(json, request);
  }
};
} // namespace WebPubSub