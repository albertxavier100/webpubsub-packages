#pragma once
#include <cstdint>

namespace webpubsub {
namespace Constants {
namespace Retry {
uint64_t MAX_RETRIES = 10;
uint64_t MAX_RETRIES_TO_GET_MAX_DELAY = 3;
float MAX_DELAY = 50.;
float DEFAULT_DELAY = 5.;
} // namespace Retry
} // namespace Constants
} // namespace webpubsub
