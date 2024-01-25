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

  bool try_get_sequence_id(uint64_t &id) {
    if (updated_) {
      id = id_;
      updated_ = true;
      return true;
    }
    id = 0;
    return false;
  }

  bool try_update(uint64_t id) {
    updated_ = true;
    if (id > id_) {
      id_ = id;
      return true;
    }
    return false;
  }

private:
  bool updated_;
  uint64_t id_;
};
} // namespace detail
} // namespace webpubsub