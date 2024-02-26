#pragma once

#include "spdlog/spdlog.h"

namespace webpubsub {
namespace detail {

namespace log_template {
constexpr char *failed_to_receive_bytes =
    "An exception occurred while receiving bytes. Error Message: {0}";
constexpr char *websocket_closed = "WebSocket connection is closed.";

} // namespace log_template

class log {
  using logger_ptr = std::shared_ptr<spdlog::logger>;

public:
  log(logger_ptr logger) : logger_(std::move(logger)) {}

  void failed_to_receive_bytes(std::string &&message) {
    logger_->info(log_template::failed_to_receive_bytes, message);
  }
  void websocket_closed() { logger_->info(log_template::websocket_closed); }

private:
  logger_ptr logger_;
};
} // namespace detail
} // namespace webpubsub