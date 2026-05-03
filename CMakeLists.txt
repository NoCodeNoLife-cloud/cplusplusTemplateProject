# CMake version requirement and standard settings
cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 26)

if (MSVC)
    add_compile_options("/source-charset:utf-8")
    add_compile_options("/execution-charset:utf-8")
    add_compile_options("/utf-8")
endif()

set(default_build_type "Debug")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

# Toolchain configuration
if (CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
else ()
    message(WARNING "No toolchain file specified. Default toolchain will be used.")
endif ()

# Project setup
set(MAIN_PROJECT cpp_project)
project(${MAIN_PROJECT})

# Include directories
include_directories(common)
include_directories(proto)
include_directories(log)
include_directories(client)
include_directories(server)
include_directories(ci)

# Vcpkg dependencies
find_package(boost CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(gtest CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(directxmath CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)

# Subdirectories
add_subdirectory(common)
add_subdirectory(proto)
add_subdirectory(log)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(ci)
