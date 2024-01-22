#pragma once
#include <WebPubSub/client/async/task_completion/task_completion_source.hpp>
namespace webpubsub {
class ack_entity {
public:
  ack_entity(uint64_t ack_id) id_(ack_id) {}

private:
  uint64_t id_;
};
} // namespace webpubsub
