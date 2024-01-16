#pragma once
namespace WebPubSub {
namespace Constants {
namespace Keys {
// common
static const char *const type = "type";

// group
static const char *const group = "group";
static const char *const noEcho = "noEcho";

// id
static const char *const ackId = "ackId";
static const char *const sequenceId = "sequenceId";

// data
static const char *const dataType = "dataType";
static const char *const data = "data";

// event
static const char *const event = "event";

// response
static const char *const success = "success";
static const char *const userId = "userId";
static const char *const connectionId = "connectionId";
static const char *const reconnectionToken = "reconnectionToken";
static const char *const from = "from";
static const char *const fromUserId = "fromUserId";

// error
static const char *const error = "error";
static const char *const name = "name";
static const char *const message = "message";
} // namespace Keys

namespace Defaults {
static const std::string emptyString = "";
}
} // namespace Constants
} // namespace WebPubSub