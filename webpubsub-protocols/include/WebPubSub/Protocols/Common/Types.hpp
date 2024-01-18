#pragma once
namespace webpubsub {
enum DataType {
  Json = 0,
  Text,
  Binary,
};

enum AckErrorType {
  Forbidden = 0,
  InternalServerError,
  Duplicate,
};

enum RequestType {
  JoinGroup = 0,
  LeaveGroup,
  SendToGroup,
  SendCustomEvent,
  SequenceAck,
};

enum ResponseType {
  UnknownResponseType = 0,
  Ack,
  Message,
  System,
};

enum SystemEventType {
  UnknownSystemEventType = 0,
  Connected,
  Disconnected,
};

enum MessageSourceType {
  UnknownMessageSourceType = 0,
  Group,
  Server,
};

enum WebPubSubProtocolMessageType {
  WebPubSubProtocolMessageText,
  WebPubSubProtocolMessageBinary,
};
} // namespace webpubsub