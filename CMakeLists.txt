# ══════════════════════════════════════════════════════════════════════════
#  Root project — cpp_project
# ══════════════════════════════════════════════════════════════════════════
#
# Top-level CMake configuration for the entire C++ project.
# Defines global compiler settings, toolchain, dependencies, and
# sub-project entry points (common, proto, log, client, server, ci).
#
# ══════════════════════════════════════════════════════════════════════════

# ── Compiler & standards ─────────────────────────────────────────────────
cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 26)

# Enable UTF-8 source/execution character set on MSVC
if (MSVC)
    add_compile_options("/source-charset:utf-8")
    add_compile_options("/execution-charset:utf-8")
    add_compile_options("/utf-8")
endif ()

# ── Build type ───────────────────────────────────────────────────────────
# Default to Debug unless the user explicitly chooses another type
set(default_build_type "Debug")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

# ── Toolchain (vcpkg) ────────────────────────────────────────────────────
# Prefer an explicit toolchain file; fall back to $VCPKG_ROOT if set
if (CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
else ()
    if (DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
        message(STATUS "Automatically detected vcpkg toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
    else ()
        message(WARNING "No toolchain file specified and VCPKG_ROOT environment variable not set. Default toolchain will be used.")
    endif ()
endif ()

# ── Project declaration ──────────────────────────────────────────────────
set(MAIN_PROJECT cpp_project)
project(${MAIN_PROJECT} VERSION 0.1.0 DESCRIPTION "cppforge" LANGUAGES CXX)

include(GNUInstallDirs)

# ── Version header ────────────────────────────────────────────────────────
configure_file(
    common/include/cppforge/Version.h.in
    common/include/cppforge/Version.h
    @ONLY
)

enable_testing()

option(BUILD_EXAMPLES "Build example applications" OFF)

# ── Package dependencies ─────────────────────────────────────────────────
# All third-party libraries are managed through vcpkg
find_package(Boost CONFIG REQUIRED COMPONENTS serialization)
find_package(glog CONFIG REQUIRED)
find_package(gtest CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
if(WIN32)
    find_package(directxmath CONFIG REQUIRED)
endif()
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

# ── Sub-projects ─────────────────────────────────────────────────────────
add_subdirectory(common)
add_subdirectory(proto)
add_subdirectory(log)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(ci)

# ── Examples ──────────────────────────────────────────────────────────────
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ── Install ───────────────────────────────────────────────────────────────
install(EXPORT cppforgeTargets
    NAMESPACE cppforge::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppforge
    FILE cppforgeTargets.cmake
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/cppforge-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cppforge-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppforge
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cppforge-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cppforge-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cppforge-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppforge
)
