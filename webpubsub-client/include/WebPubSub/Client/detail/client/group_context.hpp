#pragma once
#include <string>
namespace webpubsub {
namespace detail {

class group_context {
public:
  group_context() : joined_(false) {}
  bool is_joined() const { return joined_; }
  void set_joined(bool joined) { joined_ = joined; };

private:
  bool joined_;
};
} // namespace detail
} // namespace webpubsub