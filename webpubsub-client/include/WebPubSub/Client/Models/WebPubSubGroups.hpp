#pragma once
#include <string>
#include <unordered_map>

namespace WebPubSub {
class WebPubSubGroup {
public:
  WebPubSubGroup(std::string group) : group(std::move(group)), joined(false) {}

  bool isJoined() { return joined; }
  void setJoined(bool joinedIn) { joined = joinedIn; };

private:
  std::string group;
  bool joined;
};

class WebPubSubGroups {
public:
  WebPubSubGroup &getOrSet(const std::string &group) {
    auto it = groups.find(group);
    if (it == groups.end()) {
      it = groups.insert({group, std::move(WebPubSubGroup(group))}).first;
    }
    return it->second;
  }

  private:
    std::unordered_map<std::string, WebPubSubGroup> groups;
};
} // namespace WebPubSub