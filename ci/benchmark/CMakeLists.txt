# ══════════════════════════════════════════════════════════════════════════
#  Benchmark target — ci_benchmarks
# ══════════════════════════════════════════════════════════════════════════
#
# Single executable containing all micro-benchmarks (Google Benchmark).
# Benchmarks cover: cache, toolkit, crypto, data-structure, and tier2
# modules.
#
# Usage:
#   cmake --build <build> --target ci_benchmarks
#   ./ci_benchmarks --benchmark_filter="LRUCache"

set(CI_BENCHMARK_TARGET ci_benchmarks)

# ── Source collection ──────────────────────────────────────────────────
file(GLOB_RECURSE CI_BENCHMARK_SRC_FILES
    *.cc
    *.cpp
)

# ── Executable ─────────────────────────────────────────────────────────
add_executable(${CI_BENCHMARK_TARGET} ${CI_BENCHMARK_SRC_FILES})

# ── Include directories ────────────────────────────────────────────────
target_include_directories(${CI_BENCHMARK_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../client/src
)

# ── Linked libraries ───────────────────────────────────────────────────
target_link_libraries(${CI_BENCHMARK_TARGET} PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
    glog::glog
    yaml-cpp::yaml-cpp
    rapidjson
    OpenSSL::SSL OpenSSL::Crypto
    $<$<PLATFORM_ID:Windows>:Microsoft::DirectXMath>
    magic_enum::magic_enum
    common_pkg
    glog_service
    grpc_service
    fmt::fmt
    unofficial::sqlite3::sqlite3
    unofficial::mysql-connector-cpp::connector
)

# ── CTest registration ─────────────────────────────────────────────────
add_test(NAME ${CI_BENCHMARK_TARGET} COMMAND ${CI_BENCHMARK_TARGET})
