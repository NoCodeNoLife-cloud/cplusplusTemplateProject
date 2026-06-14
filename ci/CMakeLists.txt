# ══════════════════════════════════════════════════════════════════════════
#  CI — Continuous Integration
# ══════════════════════════════════════════════════════════════════════════
#
# Top-level CMake for all CI targets.
#
# Sub-directories:
#   - gtest/      — Google Test unit tests (per-module executables)
#   - benchmark/  — Google Benchmark performance benchmarks (single target)
#
# Usage:
#   cmake -S . -B build && cmake --build build
#   ctest --test-dir build -j "$(nproc)" -R "^ut_"

enable_testing()

add_subdirectory(gtest)
add_subdirectory(benchmark)