#pragma once
#include <variant>
#include <webpubsub/client/async/task_completion/task_completion_source.hpp>
#include <webpubsub/client/models/request_result.hpp>
namespace webpubsub {
using result_or_exception = std::variant<
    request_result,
    std::invalid_argument /* TODO: replace my own exception in the future*/,
    std::exception>;

class ack_entity__ {
public:
  ack_entity(uint64_t ack_id) id_(ack_id) {}
  auto get_ack_id() { return id_; }

  // TODO: in the future, limit set function, and consider try set...
  auto set_result(request_result result) {
    tcs_.set_value_once(std::move(result));
  }

private:
  uint64_t id_;

  task_completion_source<request_result_or_exception> tcs_;
};
} // namespace webpubsub
