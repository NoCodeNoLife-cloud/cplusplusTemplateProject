# ══════════════════════════════════════════════════════════════════════════
#  Client executable — client_app
# ══════════════════════════════════════════════════════════════════════════
#
# Main client application executable.  Communicates with the server via
# gRPC and provides the CLI / TUI user interface.
#
# ══════════════════════════════════════════════════════════════════════════

# ── Target ───────────────────────────────────────────────────────────────
set(CLIENT_PROJECT client_app)

# ── Sources ──────────────────────────────────────────────────────────────
file(GLOB_RECURSE CLIENT_SRC_FILES
    src/*.cc
    src/*.hpp
)

# ── Executable ───────────────────────────────────────────────────────────
add_executable(${CLIENT_PROJECT} ${CLIENT_SRC_FILES})

target_include_directories(${CLIENT_PROJECT} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${CLIENT_PROJECT} PRIVATE
    glog::glog
    yaml-cpp::yaml-cpp
    RapidJSON rapidjson
    OpenSSL::SSL OpenSSL::Crypto
    $<$<PLATFORM_ID:Windows>:Microsoft::DirectXMath>
    magic_enum::magic_enum
    common_pkg
    glog_service
    grpc_service
    fmt::fmt
    unofficial::sqlite3::sqlite3
)
