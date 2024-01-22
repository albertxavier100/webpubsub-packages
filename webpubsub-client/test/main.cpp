#include <gtest/gtest.h>

#include "client_tests.hpp"

int main(int argc, char **argv) {
  // Initialize Google Test
  ::testing::InitGoogleTest(&argc, argv);

  // Run tests
  RUN_ALL_TESTS();
  return 0;
}