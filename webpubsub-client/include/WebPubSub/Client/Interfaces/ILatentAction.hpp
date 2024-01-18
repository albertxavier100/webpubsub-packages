#pragma once

namespace webpubsub {
template <typename Impl> struct ILatentAction {
  void delay(float seconds)const { static_cast<Impl *>(this)->delay(seconds); }
};
} // namespace webpubsub