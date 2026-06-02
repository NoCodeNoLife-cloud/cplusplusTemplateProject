# Project
set(CI_TEST_PROJECT ci_tests)

# Source files collection
file(GLOB_RECURSE CI_TEST_SRC_FILES
        ut/*.cc
        ut/*.cpp
)

# Enable testing
enable_testing()

# Define the test executable
add_executable(${CI_TEST_PROJECT} ${CI_TEST_SRC_FILES})

target_include_directories(${CI_TEST_PROJECT} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ut
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../proto
        ${CMAKE_CURRENT_SOURCE_DIR}/../log/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../client/src
)

target_sources(${CI_TEST_PROJECT} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../client/src/auth/AuthRpcParam.cc
)

# Link required libraries
target_link_libraries(${CI_TEST_PROJECT} PRIVATE
        GTest::gtest
        GTest::gtest_main
        glog::glog
        yaml-cpp::yaml-cpp
        rapidjson
        OpenSSL::SSL OpenSSL::Crypto
        Microsoft::DirectXMath
        magic_enum::magic_enum
        common_pkg
        glog_service
        grpc_service
        fmt::fmt
        unofficial::sqlite3::sqlite3
        unofficial::mysql-connector-cpp::connector
)

# Add tests
add_test(NAME ${CI_TEST_PROJECT} COMMAND ${CI_TEST_PROJECT})
