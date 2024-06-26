cmake_minimum_required(VERSION 3.12)

project(test-webpubsub-client VERSION 0.1.0)

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-3")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -gdwarf-3")

set(SOURCES
        test/main.cpp
)

include_directories(
        ../deps/json/include
        ../deps/cppcodec
        ../deps/spdlog/include
        ../deps/uri/include/fix8
        ../deps/eventpp/include
        ../deps/uri-library
        D:/XLDownloads/test-beast/boost # set to your boost header
)

find_package(GTest CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(test-webpubsub-client ${SOURCES})
if (WIN32 AND CMAKE_C_COMPILER_ID MATCHES "MSVC" OR CMAKE_C_COMPILER_ID MATCHES "Visual Studio")
    message("Building on Windows")
    target_compile_options(test-webpubsub-client PRIVATE "/bigobj")
endif ()

target_compile_definitions(test-webpubsub-client PRIVATE ASIO_HAS_CO_AWAIT=1)
# target_compile_definitions(test-webpubsub-client PRIVATE ASIO_STANDALONE=1)
target_link_libraries(test-webpubsub-client PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
target_link_libraries(test-webpubsub-client PRIVATE OpenSSL::SSL)
target_link_libraries(test-webpubsub-client PRIVATE OpenSSL::Crypto)

target_include_directories(test-webpubsub-client PUBLIC
        include
        ../webpubsub-protocols/include
)
target_include_directories(test-webpubsub-client PUBLIC
        impl
)