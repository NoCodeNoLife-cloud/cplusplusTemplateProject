# AGENTS.md — cplusplus_project_template

A C++ project template for rapid program development (Apache 2.0).  
Minimal README — the build files and code are the source of truth.

## Project structure

```
CMakeLists.txt              # Root: C++26, vcpkg, 15+ dependencies
.clang-format               # Google-based, indent=4, ColumnLimit=600 (118 lines)
├── common/                 # Static lib `common_pkg` (192 .hpp + 106 .cc)
│   └── src/
│       ├── interface/      # 18 interfaces (ICache, IConfigurable NVI, ITask, IRunnable,
│       │                   #   IReadable, IFlushable, IYamlSerializable, etc.) with
│       │                   #   task/ io/ serialization/ subdirectories
│       ├── toolkit/        # StringToolkit, ObjectFactory<T>, registry, etc. (10 .hpp + 4 .cc)
│       ├── aop/            # AOP utilities (1 .hpp + 1 .cc)
│       ├── time/           # Date, Clock, SimpleDateFormatter (3 .hpp + 3 .cc)
│       ├── gen/            # UUID, Snowflake, Random generators (3 .hpp + 3 .cc)
│       ├── data_structure/ # 85 .hpp + 30 .cc across 20 subdirectories:
│       │                   #   base_type/, concurrent/, deque/, filter/, geometry/,
│       │                   #   graph/ + algorithm/, hash/, heap/, list/, map/,
│       │                   #   persistent/, probabilistic/, queue/, set/, spatial/,
│       │                   #   stack/, string/, top_k/, tree/, union_find/
│       ├── cache/          # Cache interface headers (2 .hpp)
│       ├── buffer/         # Buffer utilities (7 .hpp + 7 .cc)
│       ├── io/             # IO tools with reader/ and writer/ subdirs (25 .hpp + 24 .cc)
│       ├── serialization/  # Serialization tools (2 .hpp + 1 .cc)
│       ├── filesystem/     # Filesystem tools with core/ and type/ subdirs (6 .hpp + 6 .cc)
│       ├── auth/           # Authentication module (4 .hpp + 4 .cc)
│       ├── crypto/         # Crypto module with hash/ and cipher/ subdirs (12 .hpp + 11 .cc)
│       ├── system/         # System utilities (3 .hpp + 3 .cc)
│       ├── thread/         # Thread utilities (5 .hpp + 4 .cc)
│       ├── sql/            # SQL tools with sqlite/ and mysql/ subdirs (3 .hpp + 3 .cc)
│       └── rpc/            # RPC utilities (3 .hpp + 2 .cc)
├── proto/                  # gRPC codegen lib `grpc_service`
│   ├── src/RpcService.proto
│   └── generated/          # Auto-generated (gitignored)
├── log/                    # glog wrapper lib `glog_service`
├── client/                 # Executable `client_app`
│   ├── src/config/application-dev.yml  # connects to localhost:50051
│   └── src/config/ConfigParam.h        # .h suffix (only non-.hpp header in project)
├── server/                 # Executable `server_app`
│   └── src/config/application-dev.yml  # binds 0.0.0.0:50051
├── ci/
│   ├── gtest/              # Per-directory test executables (ut_*), 180+ tests across 20 dirs
│   └── benchmark/          # Single `ci_benchmarks` executable, 6 source files
├── runCfg/                 # IntelliJ run configuration XMLs (6 files)
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
Each directory → one `ut_<underscore_path>` executable (20 directories, 180+ tests).  
Shared support sources (auth/config params) compile into every test exe — expect slower first build.

Benchmark sources (6 files) in `ci/benchmark/src/`:
- `cache_benchmark.cc`, `crypto_benchmark.cc`, `data_structure_benchmark.cc`,
  `hash_benchmark.cc`, `tier2_benchmark.cc`, `toolkit_benchmark.cc`

## Proto / gRPC

Only one proto: `proto/src/RpcService.proto` (AuthService: 6 RPCs).  
Regenerate with `cmake --build build --target generate_proto`.  
Requires `grpc_cpp_plugin` from vcpkg (`$env:VCPKG_ROOT/installed/x64-windows/tools/grpc/`).  
Output goes to `proto/generated/` (gitignored; do not edit manually).

## Key conventions (non-default)

- **.clang-format**: Google base, but IndentWidth=4, BreakBeforeBraces=Attach, ColumnLimit=600,
  PointerAlignment=Left, ReferenceAlignment=Left, NamespaceIndentation=None,
  SortIncludes=CaseInsensitive, IncludeBlocks=Preserve, AccessModifierOffset=-4, Standard=C++20.
- **NVI pattern**: `IConfigurable` uses public `config()` → protected `doConfig()`.
- **Exception handling**: `StackTraceExceptionHandler::logException()` in all `main()` catch blocks.
- **Singleton access**: Meyers' singleton (`static T& getInstance()`) for config params.
- **Factory pattern**: `ObjectFactory<T>` with `registerType<Derived>(name, args...)` / `createObject(name)`.
- **Cache hierarchy**: `ICache<Key, Value>` interface; `LRUCache` & `LFUCache` implementations.
- **Thread safety**: ObjectFactory uses internal mutex (not thread-safe by default — check each class).
- **Config**: YAML (`application-dev.yml`) loaded by `ConfigParam` singleton. Paths are relative to
  the executable's working directory. `ConfigParam.h` uses `.h` suffix — the only non-`.hpp` header
  in the project.
- **glog**: All modules use glog. Both apps log to stderr with custom format by default.
- **data_structure module**: Largest module in `common/` with ~85 header files and ~30 source files
  across 20 subdirectories. Covers graph algorithms (BFS/DFS/Dijkstra/Kruskal/Prim/Bellman-Ford/
  Tarjan/Floyd-Warshall), probabilistic data structures (BloomFilter, HyperLogLog, CountMinSketch,
  TDigest, MinHash), spatial structures (RTree), persistent data structures, skip lists,
  consistent hashing, union-find with rollback, Aho-Corasick string search, and more.

## vcpkg dependencies

Boost serialization, glog, gtest, yaml-cpp, RapidJSON, OpenSSL, DirectXMath, Protobuf, gRPC, magic_enum, sqlite3, fmt, mysql-connector-cpp, benchmark.

Toolchain: set `$env:VCPKG_ROOT` or pass `-DCMAKE_TOOLCHAIN_FILE=...` on configure.

## OpenCode

This repo has OpenCode artifact caching (`.cache/cheshire/artifacts/`).  
Do not commit the `.cache/` directory (already in `.gitignore`).  
The `AGENTS.md` user instruction is read from this file.
