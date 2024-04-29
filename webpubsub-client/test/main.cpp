#include <gtest/gtest.h>

#include "client_connectivity_test_v2.hpp"
#include "detail_tests.hpp"
 #include "beast_tests.hpp"
 #include "default_websocket_tests.hpp"
#include "client_with_beast_test.hpp"

int main(int argc, char **argv) {
  // Initialize Google Test
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::GTEST_FLAG(filter) = "*auto_reconnect*";

  // Run tests
  return RUN_ALL_TESTS();
}