#include <gtest/gtest.h>

#include "client_connectivity_test_v2.hpp"
#include "detail_tests.hpp"

int main(int argc, char **argv) {
  // Initialize Google Test
  ::testing::InitGoogleTest(&argc, argv);

  // Run tests
  return RUN_ALL_TESTS();
}