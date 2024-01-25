cmake_minimum_required(VERSION 3.12)

project(test-webpubsub-client VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(../deps/vcpkg/scripts/buildsystems/vcpkg.cmake)


set(SOURCES
    test/main.cpp
)

find_package(GTest CONFIG REQUIRED)

add_executable(test-webpubsub-client ${SOURCES})
target_link_libraries(test-webpubsub-client PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
target_include_directories(test-webpubsub-client PUBLIC
    include

    ../webpubsub-protocols/include
    ../deps/json/include
    ../deps/cppcodec
    ../deps/spdlog/include
    ../deps/asio/asio/include
)
