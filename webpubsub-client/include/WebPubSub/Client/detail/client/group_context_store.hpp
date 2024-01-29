#pragma once
#include <unordered_map>
#include <webpubsub/client/detail/client/group_context.hpp>

namespace webpubsub {
namespace detail {
class group_context_store {
public:
  group_context &get_or_set(const std::string &group) {
    auto it = groups_.find(group);
    if (it == groups_.end()) {
      it = groups_.insert({group, group_context(group)}).first;
    }
    return it->second;
  }

private:
  std::unordered_map<std::string, group_context> groups_;
};
} // namespace detail
} // namespace webpubsub