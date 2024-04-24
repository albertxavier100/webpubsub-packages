//
// Created by alber on 2024/2/24.
//

#ifndef TEST_WEBPUBSUB_CLIENT_CORE_HPP
#define TEST_WEBPUBSUB_CLIENT_CORE_HPP

namespace asio {}

namespace boost {
namespace asio {}

} // namespace boost

namespace webpubsub {
namespace io {
#ifdef ASIO_STANDALONE
using namespace ::asio;
#else
using namespace ::boost::asio;
using namespace ::boost::system;
using namespace ::boost;

#endif
} // namespace io
} // namespace webpubsub

#endif // TEST_WEBPUBSUB_CLIENT_CORE_HPP
