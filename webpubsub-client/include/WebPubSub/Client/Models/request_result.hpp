#pragma once
#include <optional>

namespace webpubsub {
struct request_result {
  std::optional<uint64_t> ack_id;
  bool is_duplicated;
};
} // namespace webpubsub