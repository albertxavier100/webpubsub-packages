#pragma once
#include <string>
namespace webpubsub {
namespace detail {
class group_context_store;

class group_context {
public:
  group_context(std::string group) : group_(std::move(group)), joined_(false) {}
  bool is_joined() { return joined_; }
  void set_joined(bool joined) { joined_ = joined; };

private:
  std::string group_;
  bool joined_;
};
} // namespace detail
} // namespace webpubsub