//
// Created by alber on 2024/2/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_EXCEPTION_HPP
#define TEST_WEBPUBSUB_CLIENT_EXCEPTION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)
#include <exception>
namespace webpubsub {

// TODO: IMPL
class exception : public std::exception {
protected:
  explicit exception(std::string const &message) {}
};

// TODO: IMPL
class invalid_operation : public exception {
public:
  explicit invalid_operation(std::string const &message) : exception(message) {}
};
} // namespace webpubsub
#endif // TEST_WEBPUBSUB_CLIENT_EXCEPTION_HPP
