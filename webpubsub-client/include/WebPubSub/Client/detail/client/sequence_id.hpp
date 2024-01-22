#pragma once

namespace webpubsub {
namespace detail {
class sequence_id {
public:
  sequence_id() : updated_(false), id_(0) {}
  void reset() {
    updated_ = false;
    id_ = 0;
  }

private:
  bool updated_;
  uint64_t id_;
};
} // namespace detail
} // namespace webpubsub