//
// Created by alber on 2024/3/7.
//

#ifndef TEST_WEBPUBSUB_CLIENT_RETRY_MODE_HPP
#define TEST_WEBPUBSUB_CLIENT_RETRY_MODE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace webpubsub {
enum class retry_mode { fixed, exponential };
}

#endif // TEST_WEBPUBSUB_CLIENT_RETRY_MODE_HPP
