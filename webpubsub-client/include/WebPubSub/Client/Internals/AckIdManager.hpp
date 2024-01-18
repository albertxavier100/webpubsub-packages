#pragma once
#include <iostream>
#include <unordered_map>

namespace webpubsub {
class AckEntity {
public:
  AckEntity(uint64_t ackId) : ackId(ackId) {}

private:
  uint64_t getAckId() { return ackId; }

private:
  uint64_t ackId;
};
} // namespace webpubsub

namespace webpubsub {
class AckIdManager {
public:
  AckIdManager() : nextAckId(0) {}
  uint64_t next() { return nextAckId++; }

private:
  uint64_t nextAckId;
  std::unordered_map<uint64_t, AckEntity> store;
};
} // namespace webpubsub