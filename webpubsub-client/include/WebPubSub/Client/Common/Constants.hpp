#pragma once
#include <cstdint>

namespace webpubsub {
namespace constants {
namespace retry {
uint64_t MAX_RETRIES = 10;
uint64_t MAX_RETRIES_TO_GET_MAX_DELAY = 3;
float MAX_DELAY = 50.;
float DEFAULT_DELAY = 5.;
} // namespace retry

namespace query {
const char recover_connection_id_query_[] = "awps_connection_id";
const char recover_connection_token_query_[] = "awps_reconnection_token";
} // namespace query
} // namespace constants
} // namespace webpubsub
