//
// Created by alber on 2024/2/24.
//

#ifndef TEST_WEBPUBSUB_CLIENT_TASK_TRACKER_HPP
#define TEST_WEBPUBSUB_CLIENT_TASK_TRACKER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "webpubsub/client/common/asio.hpp"

#ifdef ASIO_STANDALONE

#include "asio/cancellation_signal.hpp"
#include "asio/steady_timer.hpp"
#endif

#endif // TEST_WEBPUBSUB_CLIENT_TASK_TRACKER_HPP
