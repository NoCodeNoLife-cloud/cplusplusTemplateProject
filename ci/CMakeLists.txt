# Project
set(CI_TEST_PROJECT ci_tests)

# Source files collection
file(GLOB_RECURSE CI_TEST_SRC_FILES
        ut/src/*.cc
        ut/src/*.cpp
)

# Enable testing
enable_testing()

# Define the test executable
add_executable(${CI_TEST_PROJECT} ${CI_TEST_SRC_FILES})

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
)

# Add tests
add_test(NAME ${CI_TEST_PROJECT} COMMAND ${CI_TEST_PROJECT})
