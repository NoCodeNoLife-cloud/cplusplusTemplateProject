# AGENTS.md — C++ Project Template

## Build system

- CMake 3.30+, C++26 (`CMAKE_CXX_STANDARD 26`).
- Default config is `Debug`. Override with `-DCMAKE_BUILD_TYPE=Release` etc.
- **Requires `VCPKG_ROOT` env var** or explicit `CMAKE_TOOLCHAIN_FILE`. Without it, `find_package` calls for all dependencies (Boost, gRPC, Protobuf, glog, gtest, OpenSSL, etc.) will fail.
- MSVC gets `/utf-8` flags automatically from root `CMakeLists.txt`.
- Build artifacts: `cmake-build-*/` (gitignored).

## Project structure

```
common/       → library  `common_pkg`   (shared utilities, crypto, IO, time, threading, toolkit, etc.)
proto/        → library  `grpc_service`  (gRPC+Protobuf codegen)
log/          → library  `glog_service`  (glog wrapper)
client/       → exe      `client_app`    (entry: client/src/main.cc)
server/       → exe      `server_app`    (entry: server/src/main.cc)
ci/gtest/     → exe      `ci_gtests`     (Google Test)
ci/benchmark/ → exe      `ci_benchmarks` (Google Benchmark)
runCfg/       → CLion run configurations
script/       → clean_work_space.py
```

## Build & run

```
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug --target <target>

# Targets: client_app, server_app, ci_gtests, ci_benchmarks, generate_proto
```

Run tests via CTest:
```
cmake --build cmake-build-debug --target ci_gtests
cd cmake-build-debug && ctest --output-on-failure
```

## Proto codegen

- Input: `proto/src/*.proto` → Output: `proto/generated/`.
- Target `generate_proto` runs `protoc` + `grpc_cpp_plugin` (auto-found via vcpkg).
- Generation happens at build time. The `grpc_service` library depends on `generate_proto`.

## Code conventions

- `.hpp` headers, `#pragma once`
- `.cc` sources
- Namespaces mirror package: `client_app::task`, `server_app::config`, `common::toolkit`
- Formatting: Google style with 4-space indent, left `*`/`&`, 600-char column limit (`.clang-format`)
- Doxygen `@file`/`@brief`/`@details` on all headers
- `[[nodiscard]]` on accessors and factory methods

## Config

- YAML via `yaml-cpp`: `client/src/config/application-dev.yml`, `server/src/config/application-dev.yml`
- Config paths are relative: `../../client/src/config/application-dev.yml` (set at compile time in `ConfigParam`)

## Testing

- Google Test (gtest) via vcpkg.
- Tests use `TEST_F` with test fixtures. `using namespace` in test files is accepted.
- One test file: `ci_gtests` — runs all test suites under `ci/gtest/src/`.
- Benchmarks use Google Benchmark, main provided by `benchmark::benchmark_main`.
