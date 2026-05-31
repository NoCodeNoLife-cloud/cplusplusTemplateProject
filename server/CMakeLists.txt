# Project
set(SERVER_PROJECT server_app)

# Source files collection
file(GLOB_RECURSE SERVER_SRC_FILES
        src/*.cc
        src/*.hpp
)

# Define the executable and its source files and framework files
add_executable(${SERVER_PROJECT} ${SERVER_SRC_FILES})

target_include_directories(${SERVER_PROJECT} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../proto
        ${CMAKE_CURRENT_SOURCE_DIR}/../log/src
)

# Link required libraries
target_link_libraries(${SERVER_PROJECT} PRIVATE
        glog::glog
        yaml-cpp::yaml-cpp
        RapidJSON rapidjson
        OpenSSL::SSL OpenSSL::Crypto
        Microsoft::DirectXMath
        magic_enum::magic_enum
        common_pkg
        glog_service
        grpc_service
        fmt::fmt
        unofficial::sqlite3::sqlite3
)
