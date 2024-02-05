#pragma once
#include <WebPubSub/Protocols/Common/Types.hpp>
#include <WebPubSub/Protocols/Responses/ConnectedResponse.hpp>
#include <WebPubSub/Protocols/Responses/DisconnectedResponse.hpp>
#include <exception>
#include <optional>
#include <string>

namespace webpubsub {
struct connected_context {
  std::string connection_id;
  std::optional<std::string> user_id;
  std::optional<std::string> reconnection_token;
};

struct disconnected_context {
  std::optional<std::string> connection_id;
  std::optional<std::string> reason;
};

// TODO: impl
struct group_data_context {
  std::string group;
  std::optional<uint64_t> sequence_id;
  std::optional<std::string> from_user_id;
  DataType data_type;
  // TODO data
};

// TODO: impl
struct server_data_context {
  std::optional<uint64_t> sequence_id;
  DataType data_type;
  // TODO data
};

struct rejoin_group_failed_context {
  std::string group;
  std::exception exception;
};

struct stopped_context {};

} // namespace webpubsub
