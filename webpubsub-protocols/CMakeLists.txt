cmake_minimum_required(VERSION 3.12)

project(test-webpubsub-protocols VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES
    test/main.cpp
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(test-webpubsub-protocols ${SOURCES})
target_link_libraries(test-webpubsub-protocols GTest::gtest_main)
target_include_directories(test-webpubsub-protocols PUBLIC
    include
    ../deps/json/include
    ../deps/cppcodec
)