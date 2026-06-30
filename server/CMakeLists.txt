# ══════════════════════════════════════════════════════════════════════════
#  Server executable — server_app
# ══════════════════════════════════════════════════════════════════════════
#
# Main server application executable.  Hosts gRPC services, manages
# client connections, and coordinates all backend operations.
#
# ══════════════════════════════════════════════════════════════════════════

# ── Target ───────────────────────────────────────────────────────────────
set(SERVER_PROJECT server_app)

# ── Sources ──────────────────────────────────────────────────────────────
file(GLOB_RECURSE SERVER_SRC_FILES
    src/*.cc
    src/*.hpp
)

# ── Executable ───────────────────────────────────────────────────────────
add_executable(${SERVER_PROJECT} ${SERVER_SRC_FILES})

target_include_directories(${SERVER_PROJECT} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${SERVER_PROJECT} PRIVATE
    glog::glog
    yaml-cpp::yaml-cpp
    RapidJSON rapidjson
    OpenSSL::SSL OpenSSL::Crypto
    $<$<PLATFORM_ID:Windows>:Microsoft::DirectXMath>
    magic_enum::magic_enum
    common_pkg
    glog_service
    starter_auth
    fmt::fmt
    unofficial::sqlite3::sqlite3
)
