# Project
set(CI_GTEST_PROJECT ci_gtests)

# Source files collection
file(GLOB_RECURSE CI_GTEST_SRC_FILES
        *.cc
        *.cpp
)

# Define the test executable
add_executable(${CI_GTEST_PROJECT} ${CI_GTEST_SRC_FILES})

target_include_directories(${CI_GTEST_PROJECT} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../common/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../proto
        ${CMAKE_CURRENT_SOURCE_DIR}/../../log/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../client/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../server/src
)

target_sources(${CI_GTEST_PROJECT} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../client/src/auth/AuthRpcParam.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../server/src/auth/AuthRpcParam.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../server/src/config/ConfigParam.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../client/src/config/ConfigParam.cc
)

# Link required libraries
target_link_libraries(${CI_GTEST_PROJECT} PRIVATE
        GTest::gtest
        GTest::gtest_main
        glog::glog
        yaml-cpp::yaml-cpp
        rapidjson
        OpenSSL::SSL OpenSSL::Crypto
        Microsoft::DirectXMath
        magic_enum::magic_enum
        Boost::serialization
        common_pkg
        glog_service
        grpc_service
        fmt::fmt
        unofficial::sqlite3::sqlite3
        unofficial::mysql-connector-cpp::connector
)

# Add tests
add_test(NAME ${CI_GTEST_PROJECT} COMMAND ${CI_GTEST_PROJECT})
