#pragma once
#include <optional>

namespace webpubsub {
struct request_result {
  std::optional<uint64_t> ack_id = std::nullopt;
  bool is_duplicated = false;
};
} // namespace webpubsub