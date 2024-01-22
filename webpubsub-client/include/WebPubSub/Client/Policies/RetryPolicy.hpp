#pragma once
#include <WebPubSub/Client/Common/Constants.hpp>
#include <chrono>
#include <optional>

// TODO: move to new file
namespace webpubsub {
struct retry_context {
  uint64_t retry_attempt;
};
} // namespace webpubsub

// TODO: move to new file
namespace webpubsub {
enum RetryMode {
  Expotential,
  Fixed,
};
} // namespace webpubsub

namespace webpubsub {
class retry_policy {
public:
  retry_policy()
      : max_retries(Constants::Retry::MAX_RETRIES),
        maxDelay(Constants::Retry::MAX_DELAY),
        maxRetriesToGetMaxDelay(Constants::Retry::MAX_RETRIES_TO_GET_MAX_DELAY),
        retryMode(Expotential), delay(Constants::Retry::DEFAULT_DELAY){};
  std::optional<float> next_retry_delay(retry_context retryContext) const {
    if (retryContext.retry_attempt > max_retries) {
      return std::nullopt;
    }
    return getDelay(retryContext.retry_attempt);
  }

private:
  float getDelay(uint64_t attempted) const {
    // TODO
    return -1;
    return calculateExponetialDelay(attempted);
  }

  float calculateExponetialDelay(uint64_t attempted) const {
    // TODO
    return -1;
  }

private:
  const uint64_t max_retries;
  const float maxDelay;
  const uint64_t maxRetriesToGetMaxDelay;
  const RetryMode retryMode;
  const float delay;
};
} // namespace webpubsub