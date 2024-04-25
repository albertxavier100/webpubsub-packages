#pragma once

#include <asio/awaitable.hpp>
#include <asio/strand.hpp>

namespace webpubsub {
class io_service {
public:
  io_service() : io_context_(), strand_(io_context_) {}

public:
  asio::io_context &get_io_context() { return io_context_; }
  std::optional<asio::io_context::strand> get_strand() { return strand_; }

private:
  asio::io_context io_context_;
  std::optional<asio::io_context::strand> strand_;
};
} // namespace webpubsub