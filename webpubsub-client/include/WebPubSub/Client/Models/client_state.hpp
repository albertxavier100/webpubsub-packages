#pragma once

namespace webpubsub {
class client_state {
public:
  enum state {
    stopped,
    connecting,
    connected,
    recovering,
  };

public:
  client_state() : state_(stopped) {}
  void change_state(state state) { state_ = state; }
  state get_state() { return state_; }

private:
  state state_;
};
} // namespace webpubsub