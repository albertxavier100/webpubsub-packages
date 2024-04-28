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

struct group_data_context {
  GroupMessageResponseV2 message;
};

struct server_data_context {
  ServerMessageResponse message;
};

struct rejoin_group_failed_context {
  std::string group;
  std::exception exception;
};

struct stopped_context {};

} // namespace webpubsub
