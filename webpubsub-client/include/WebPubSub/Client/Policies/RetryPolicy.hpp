#pragma once
#include <WebPubSub/Client/Common/Constants.hpp>
#include <chrono>
#include <optional>

// TODO: move to new file
namespace WebPubSub {
struct RetryContext {
  uint64_t retryAttempt;
};
} // namespace WebPubSub

// TODO: move to new file
namespace WebPubSub {
enum RetryMode {
  Expotential,
  Fixed,
};
} // namespace WebPubSub

namespace WebPubSub {
class RetryPolicy {
public:
  RetryPolicy()
      : maxRetries(Constants::Retry::MAX_RETRIES),
        maxDelay(Constants::Retry::MAX_DELAY),
        maxRetriesToGetMaxDelay(Constants::Retry::MAX_RETRIES_TO_GET_MAX_DELAY),
        retryMode(Expotential), delay(Constants::Retry::DEFAULT_DELAY){};
  std::optional<float> nextRetryDelay(RetryContext retryContext) const {
    if (retryContext.retryAttempt > maxRetries) {
      return std::nullopt;
    }
    return getDelay(retryContext.retryAttempt);
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
  const uint64_t maxRetries;
  const float maxDelay;
  const uint64_t maxRetriesToGetMaxDelay;
  const RetryMode retryMode;
  const float delay;
};
} // namespace WebPubSub