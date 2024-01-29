#pragma once
#include <chrono>
#include <optional>
#include <webpubsub/client/common/constants.hpp>

// TODO: move to new file
namespace webpubsub {
struct retry_context {
  uint64_t retry_attempt;
};
} // namespace webpubsub

namespace webpubsub {
class retry_policy {
public:
  enum retry_mode {
    expotential,
    fixed,
  };

public:
  retry_policy()
      : max_retries_(constants::retry::MAX_RETRIES),
        max_delay_(constants::retry::MAX_DELAY),
        max_retries_to_get_max_delay_(
            constants::retry::MAX_RETRIES_TO_GET_MAX_DELAY),
        retry_mode_(expotential), delay_(constants::retry::DEFAULT_DELAY){};
  std::optional<float> next_retry_delay(retry_context retry_context) const {
    if (retry_context.retry_attempt > max_retries_) {
      return std::nullopt;
    }
    return get_delay(retry_context.retry_attempt);
  }

private:
  float get_delay(uint64_t attempted) const {
    // TODO
    return -1;
    return calculate_exponetial_delay(attempted);
  }

  float calculate_exponetial_delay(uint64_t attempted) const {
    // TODO
    return -1;
  }

private:
  const uint64_t max_retries_;
  const float max_delay_;
  const uint64_t max_retries_to_get_max_delay_;
  const retry_mode retry_mode_;
  const float delay_;
};
} // namespace webpubsub