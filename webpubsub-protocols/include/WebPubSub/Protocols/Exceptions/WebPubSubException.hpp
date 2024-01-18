#pragma once
#include <stdexcept>

namespace webpubsub {
class WebPubSubException : public std::exception {
private:
  /* data */
public:
  WebPubSubException(/* args */) {}
  ~WebPubSubException(){}
};
} // namespace webpubsub