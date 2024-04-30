//
// Created by alber on 2024/3/25.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CLIENT_HPP
#define TEST_WEBPUBSUB_CLIENT_CLIENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/client_core.hpp"
#include "webpubsub/client/websocket/default_websocket_factory.hpp"

namespace webpubsub {
//template <typename protocol_t> class client : client_core<protocol_t, default_websocket_factory, default_websocket> {
//public:
//  client(io::strand<io::io_context::executor_type> &strand,
//         std::string client_access_uri)
//      : client_core(strand, ) {}
//
//private:
//};
} // namespace webpubsub
#endif