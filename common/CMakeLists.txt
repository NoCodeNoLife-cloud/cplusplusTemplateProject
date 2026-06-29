# ══════════════════════════════════════════════════════════════════════════
#  Common library — common_pkg
# ══════════════════════════════════════════════════════════════════════════
#
# Shared utility library consumed by client, server, and log.
# Contains data structures, algorithms, serialization helpers, and other
# cross-cutting components.
#
# ══════════════════════════════════════════════════════════════════════════

# ── Target ───────────────────────────────────────────────────────────────
set(COMMON_FRAMEWORK common_pkg)

# ── Sources ──────────────────────────────────────────────────────────────
file(GLOB_RECURSE COMMON_SRC_FILES
    src/*.cc
)

# ── Library ──────────────────────────────────────────────────────────────
add_library(${COMMON_FRAMEWORK} ${COMMON_SRC_FILES})

target_include_directories(${COMMON_FRAMEWORK} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${COMMON_FRAMEWORK} PRIVATE
    Boost::headers
    yaml-cpp::yaml-cpp
    rapidjson
    OpenSSL::SSL OpenSSL::Crypto
    magic_enum::magic_enum
    $<$<PLATFORM_ID:Windows>:Microsoft::DirectXMath>
    unofficial::sqlite3::sqlite3
    fmt::fmt
    glog::glog
    unofficial::mysql-connector-cpp::connector
)