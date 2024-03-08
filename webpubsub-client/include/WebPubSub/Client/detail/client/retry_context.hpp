//
// Created by alber on 2024/3/7.
//

#ifndef TEST_WEBPUBSUB_CLIENT_RETRY_CONTEXT_HPP
#define TEST_WEBPUBSUB_CLIENT_RETRY_CONTEXT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace webpubsub {
namespace detail {
struct retry_context {
  int attempts;
};
} // namespace detail
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_RETRY_CONTEXT_HPP
