# Project
set(CI_BENCHMARK_PROJECT ci_benchmarks)

# Source files collection
file(GLOB_RECURSE CI_BENCHMARK_SRC_FILES
        *.cc
        *.cpp
)

# Define the benchmark executable
add_executable(${CI_BENCHMARK_PROJECT} ${CI_BENCHMARK_SRC_FILES})

target_include_directories(${CI_BENCHMARK_PROJECT} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../common/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../proto
        ${CMAKE_CURRENT_SOURCE_DIR}/../../log/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../client/src
)

# Link required libraries
target_link_libraries(${CI_BENCHMARK_PROJECT} PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main
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
add_test(NAME ${CI_BENCHMARK_PROJECT} COMMAND ${CI_BENCHMARK_PROJECT})
