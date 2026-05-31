# Project
set(COMMON_FRAMEWORK common_pkg)

file(GLOB_RECURSE COMMON_SRC_FILES
        src/*.cc
        src/*.hpp
)

# Creating a Common Library Target
add_library(${COMMON_FRAMEWORK} ${COMMON_SRC_FILES})

# Set the header file paths
target_include_directories(${COMMON_FRAMEWORK} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link required libraries
target_link_libraries(${COMMON_FRAMEWORK} PRIVATE
        Boost::headers
        yaml-cpp::yaml-cpp
        rapidjson
        OpenSSL::SSL OpenSSL::Crypto
        magic_enum::magic_enum
        Microsoft::DirectXMath
        unofficial::sqlite3::sqlite3
        fmt::fmt
        glog::glog
        glog_service
        unofficial::mysql-connector-cpp::connector
)