#pragma once

#include "spdlog/spdlog.h"

namespace webpubsub {
namespace detail {

namespace log_template {
constexpr const char *failed_to_receive_bytes =
    "An exception occurred while receiving bytes. Error Message: {0}";
constexpr const char *websocket_closed = "WebSocket connection is closed.";

} // namespace log_template

class log {
  using logger_ptr = std::shared_ptr<spdlog::logger>;

public:
  log(const std::string &logger_name) : logger_(spdlog::get(logger_name)) {}

  void failed_to_receive_bytes(std::string &&message) {
    logger_->info(log_template::failed_to_receive_bytes, message);
  }
  void websocket_closed() { logger_->info(log_template::websocket_closed); }

private:
  logger_ptr logger_;
};
} // namespace detail
} // namespace webpubsub