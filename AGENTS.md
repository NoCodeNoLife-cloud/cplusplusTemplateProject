# AGENTS.md — cplusplus_project_template

A C++ project template for rapid program development (Apache 2.0).  
Minimal README — the build files and code are the source of truth.

## Project structure

```
CMakeLists.txt              # Root: C++26, vcpkg, 15+ dependencies
.clang-format               # Google-based, indent=4, ColumnLimit=600
├── common/                 # Static lib `common_pkg`
│   └── src/
│       ├── interface/      # ICache, IConfigurable(NVI), ITask, IRunnable, etc.
│       ├── toolkit/        # StringToolkit, ObjectFactory<T>, registry pattern
│       ├── time/           # Date, Clock, SimpleDateFormatter
│       └── gen/            # UUID, Snowflake, Random generators
├── proto/                  # gRPC codegen lib `grpc_service`
│   ├── src/RpcService.proto
│   └── generated/          # Auto-generated (gitignored)
├── log/                    # glog wrapper lib `glog_service`
├── client/                 # Executable `client_app`
│   └── src/config/application-dev.yml  # connects to localhost:50051
├── server/                 # Executable `server_app`
│   └── src/config/application-dev.yml  # binds 0.0.0.0:50051
├── ci/
│   ├── gtest/              # Per-directory test executables (ut_*), ~100+ tests
│   └── benchmark/          # Single `ci_benchmarks` executable
└── script/clean_work_space.py  # Removes cmake-build-* dirs
```

## Build

```powershell
# Configure (vcpkg via $env:VCPKG_ROOT or explicit CMAKE_TOOLCHAIN_FILE)
cmake -S . -B build

# Build all
cmake --build build

# Build specific target
cmake --build build --target server_app
cmake --build build --target client_app
cmake --build build --target ci_benchmarks
```

Default is `Debug`. MSVC gets `/utf-8` automatically. C++26.

## Tests

```powershell
# All unit tests (parallel, one exe per module directory)
ctest --test-dir build -j 16 -R "^ut_" --extra-verbose

# Single test module
ctest --test-dir build -R "^ut_toolkit$" --extra-verbose

# Benchmarks
.\build\ci\benchmark\Release\ci_benchmarks.exe --benchmark_filter="LRUCache"
```

Test executables are auto-discovered by directory under `ci/gtest/src/`.  
Each directory → one `ut_<underscore_path>` executable.  
Shared support sources (auth/config params) compile into every test exe — expect slower first build.

## Proto / gRPC

Only one proto: `proto/src/RpcService.proto` (AuthService: 6 RPCs).  
Regenerate with `cmake --build build --target generate_proto`.  
Requires `grpc_cpp_plugin` from vcpkg (`$env:VCPKG_ROOT/installed/x64-windows/tools/grpc/`).  
Output goes to `proto/generated/` (gitignored; do not edit manually).

## Key conventions (non-default)

- **.clang-format**: Google base, but indent=4, Attach braces, ColumnLimit=600, PointerAlignment=Left, NamespaceIndentation=None, SortIncludes=CaseInsensitive+Preserve.
- **NVI pattern**: `IConfigurable` uses public `config()` → protected `doConfig()`.
- **Exception handling**: `StackTraceExceptionHandler::logException()` in all `main()` catch blocks.
- **Singleton access**: Meyers' singleton (`static T& getInstance()`) for config params.
- **Factory pattern**: `ObjectFactory<T>` with `registerType<Derived>(name, args...)` / `createObject(name)`.
- **Cache hierarchy**: `ICache<Key, Value>` interface; `LRUCache` & `LFUCache` implementations.
- **Thread safety**: ObjectFactory uses internal mutex (not thread-safe by default — check each class).
- **Config**: YAML (`application-dev.yml`) loaded by `ConfigParam` singleton. Paths are relative to the executable's working directory.
- **glog**: All modules use glog. Both apps log to stderr with custom format by default.

## vcpkg dependencies

Boost serialization, glog, gtest, yaml-cpp, RapidJSON, OpenSSL, DirectXMath, Protobuf, gRPC, magic_enum, sqlite3, fmt, mysql-connector-cpp, benchmark.

Toolchain: set `$env:VCPKG_ROOT` or pass `-DCMAKE_TOOLCHAIN_FILE=...` on configure.

## OpenCode

This repo has OpenCode artifact caching (`.cache/cheshire/artifacts/`).  
Do not commit the `.cache/` directory (already in `.gitignore`).  
The `AGENTS.md` user instruction is read from this file.
