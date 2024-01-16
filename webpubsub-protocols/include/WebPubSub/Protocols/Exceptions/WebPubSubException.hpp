#pragma once
#include <stdexcept>

namespace WebPubSub {
class WebPubSubException : public std::exception {
private:
  /* data */
public:
  WebPubSubException(/* args */) {}
  ~WebPubSubException(){}
};
} // namespace WebPubSub