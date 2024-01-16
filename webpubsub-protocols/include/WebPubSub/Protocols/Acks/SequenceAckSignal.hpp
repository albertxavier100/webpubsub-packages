#pragma once
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <nlohmann/json.hpp>
#include <optional>

namespace WebPubSub {
class SequenceAckSignal {
public:
  SequenceAckSignal() = default;
  SequenceAckSignal(const uint64_t sequenceId) : sequenceId(sequenceId) {}

private:
  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SequenceAckSignal, type,
                                              sequenceId)

private:
  RequestType type = SequenceAck;
  uint64_t sequenceId = 0;
};
} // namespace WebPubSub