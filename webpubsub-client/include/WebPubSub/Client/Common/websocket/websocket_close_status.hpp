#pragma once

namespace webpubsub {
// TODO: [HIGH] complete from
// https://learn.microsoft.com/en-us/dotnet/api/system.net.websockets.websocketclosestatus?view=net-8.0
enum class websocket_close_status {
  empty = 1005,
  normal_close = 1000,
  policy_violation = 1008,

};
} // namespace webpubsub