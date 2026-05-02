# Project
set(COMMON_FRAMEWORK common_pkg)

file(GLOB_RECURSE COMMON_SRC_FILES
        src/*.cc
)

# Creating a Common Library Target
add_library(${COMMON_FRAMEWORK} ${COMMON_SRC_FILES})

# Set the header file paths
target_include_directories(${COMMON_FRAMEWORK} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link required libraries
target_link_libraries(${COMMON_FRAMEWORK} PUBLIC
        Boost::headers
        yaml-cpp::yaml-cpp
        rapidjson
        OpenSSL::SSL OpenSSL::Crypto
        magic_enum::magic_enum
        Microsoft::DirectXMath
        unofficial::sqlite3::sqlite3
        fmt::fmt
)