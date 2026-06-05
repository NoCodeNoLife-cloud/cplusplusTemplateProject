# AGENTS.md — C++ Project Template

## Build system

- CMake 3.30+, C++26 (`CMAKE_CXX_STANDARD 26`).
- **vcpkg**: auto-detected from `$VCPKG_ROOT`. Without it, all `find_package` calls fail.
- Dependencies (all via vcpkg): Boost (serialization), glog, gtest, yaml-cpp, RapidJSON, OpenSSL, DirectXMath, Protobuf, gRPC, magic_enum, sqlite3, fmt, mysql-connector-cpp, benchmark.
- Default build type: `Debug`. Override with `-DCMAKE_BUILD_TYPE=Release`.
- MSVC gets `/utf-8` flags automatically.
- Build artifacts: `cmake-build-*/` (gitignored).
- Clean workspace: `python script/clean_work_space.py` (removes `cmake-build-*` and `proto/generated/`).

## Project structure

```
common/       → library `common_pkg`   — framework library (226+ files: crypto, IO, containers, SQL, threading, time, serialization, etc.)
proto/        → library `grpc_service`  — gRPC+Protobuf codegen (target `generate_proto`)
log/          → library `glog_service`  — glog wrapper with YAML config
client/       → exe   `client_app`     — entry: client/src/main.cc (client_app::task::ClientTask)
server/       → exe   `server_app`     — entry: server/src/main.cc (server_app::task::ServerTask)
ci/gtest/     → exe   `ci_gtests`      — 65+ Google Test files across all modules
ci/benchmark/ → exe   `ci_benchmarks`  — Google Benchmark (6 files)
runCfg/       → CLion run configurations
script/       → clean_work_space.py
```

## Build & run

```bash
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug --target <target> [--parallel]

# Targets: client_app, server_app, ci_gtests, ci_benchmarks, generate_proto
```

Run tests:
```bash
cmake --build cmake-build-debug --target ci_gtests
cd cmake-build-debug && ctest --output-on-failure
```

Benchmarks are also registered as CTest tests. Run them the same way with `ci_benchmarks`.

## Proto codegen

- Input: `proto/src/*.proto` → Output: `proto/generated/`.
- Target `generate_proto` (build-time, marked `ALL`). The `grpc_service` library depends on it.
- Plugin auto-found at `$VCPKG_ROOT/installed/x64-windows/tools/grpc/grpc_cpp_plugin`.

## Code conventions

- Headers: `.hpp` (or `.h`), `#pragma once`.
- Sources: `.cc`.
- Namespaces mirror package structure: `client_app::task`, `server_app::task`, `server_app::auth`, `common::*` (crypto, io, thread, time, toolkit, sql, container, etc.), `glog::*`.
- Formatting: Google-derived, 4-space indent, left `*`/`&`, 600-char column limit (`.clang-format`). **Note:** `.clang-format` specifies `BreakBeforeBraces: Attach`, but existing code consistently uses Allman (braces on new lines) — be aware of this mismatch when editing.
- Doxygen `@file`/`@brief`/`@details` on all headers.
- `[[nodiscard]]` on accessors and pure functions.
- CMake files use `file(GLOB_RECURSE ...)` for source collection.

## Config

- YAML via `yaml-cpp`, parsed at startup by `ConfigParam`:
  - `client/src/config/application-dev.yml` (gRPC server address, glog settings)
  - `server/src/config/application-dev.yml` (gRPC server address, keepalive, glog settings)

## Testing

- Google Test (gtest) via vcpkg.
- Tests use `TEST_F` with fixtures. `using namespace` in test files is accepted.
- **65+ test files** under `ci/gtest/src/`, organized by module (mirrors `common/` structure).
- Test binary also compiles `client/src/auth/AuthRpcParam.cc` for client integration tests.
- MySQL executor tests exist (`MySqlExecutorTest.cc`) — may require a running MySQL instance.
- Benchmarks (6 files) under `ci/benchmark/src/`, main provided by `benchmark::benchmark_main`.
