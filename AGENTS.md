# AGENTS.md — cplusplus_project_template

A C++ project template for rapid program development (Apache 2.0).  
Minimal README — the build files and code are the source of truth.

## Project structure

```
CMakeLists.txt              # Root: C++26, vcpkg, 14 third-party dependencies
.clang-format               # Google-based, indent=4, ColumnLimit=600 (118 lines)
.gitignore                  # 41 lines: builds, IDE, generated code, OpenCode cache
.gitattributes              # Git attributes (LFS, merge strategies)
├── common/                 # Static lib `common_pkg` (192 .hpp + 106 .cc)
│   └── src/
│       ├── interface/      # 18 interfaces with 3 subdirectories:
│       │                   #   io/ (IAppendable, IBuffer, ICloseable, IFlushable, IReadable)
│       │                   #   serialization/ (IBoostSerializable, IJsonSerializable, ISerializer,
│       │                   #                 IYamlConfigurable, IYamlSerializable)
│       │                   #   task/ (IApplicationExecutor, IRunnable, IStartupTask, ITask, ITimerTask)
│       ├── toolkit/        # StringToolkit, ObjectFactory<T>, StackTraceExceptionHandler,
│       │                   #   ArraysToolkit, ClassToolkit, EnumToolkit, IntegerToolkit,
│       │                   #   RadixToolkit, RegexToolkit, StaticObjectWrapper (10 .hpp + 4 .cc)
│       ├── aop/            # FunctionProfilerAspect (1 .hpp + 1 .cc)
│       ├── time/           # Date, Clock, SimpleDateFormatter (3 .hpp + 3 .cc)
│       ├── gen/            # UUID, Snowflake, Random generators (3 .hpp + 3 .cc)
│       ├── data_structure/ # 85 .hpp + 30 .cc across 20 subdirectories:
│       │                   #   base_type/, concurrent/, deque/, filter/, geometry/,
│       │                   #   graph/ + algorithm/ (12 algorithm pairs), hash/, heap/,
│       │                   #   list/, map/, persistent/, probabilistic/, queue/, set/,
│       │                   #   spatial/, stack/, string/, top_k/, tree/ (8 sub-subdirs:
│       │                   #   balanced/, core/, crypto/, lsm/, multiway/, node/,
│       │                   #   range/, spatial/), union_find/
│       ├── cache/          # Header-only (2 .hpp)
│       ├── buffer/         # ByteBuffer, CharBuffer, etc. (7 .hpp + 7 .cc)
│       ├── io/             # Console + reader (14 .hpp + 14 .cc) + writer (10 .hpp + 9 .cc)
│       ├── serialization/  # JsonObjectSerializer, YamlObjectSerializer (2 .hpp + 1 .cc)
│       ├── filesystem/     # core/ (Directory, File, Path) + type/ (BmpImage, CsvFile, YamlToolkit) (6 .hpp + 6 .cc)
│       ├── auth/           # AuthenticationException, PasswordPolicy, UserAuthenticator, UserCredentials (4 .hpp + 4 .cc)
│       ├── crypto/         # CryptoToolKit, OpenSSLToolkit + cipher/ + hash/ (12 .hpp + 11 .cc)
│       ├── system/         # FunctionProfiler, SystemInfo, SystemPerformanceMonitor (3 .hpp + 3 .cc)
│       ├── thread/         # AutoJoinThread, ThreadPool, etc. (5 .hpp + 4 .cc)
│       ├── sql/            # PasswordSQL + sqlite/ + mysql/ (3 .hpp + 3 .cc)
│       └── rpc/            # GrpcConnectivityManager, RpcMetadata (3 .hpp + 2 .cc)
├── proto/                  # gRPC codegen lib `grpc_service`
│   ├── src/RpcService.proto
│   └── generated/          # .pb.h, .pb.cc, .grpc.pb.h, .grpc.pb.cc (gitignored)
├── log/                    # glog wrapper lib `glog_service`
│   └── src/
│       ├── config/         # GLogConfigurator
│       ├── formatter/      # PrefixFormatter
│       └── param/          # GLogParam
├── client/                 # Executable `client_app`
│   └── src/
│       ├── main.cc
│       ├── config/         # ConfigParam (.h suffix), ConfigParam.cc, application-dev.yml
│       ├── auth/           # AuthRpcParam, AuthRpcService
│       └── task/           # ClientTask
├── server/                 # Executable `server_app`
│   └── src/
│       ├── main.cc
│       ├── config/         # ConfigParam (.hpp), ConfigParam.cc, application-dev.yml
│       ├── auth/           # AuthRpcParam, AuthRpcService
│       └── task/           # ServerTask
├── ci/
│   ├── gtest/              # 20 dirs → 20 ut_* executables, ~180+ tests
│   └── benchmark/          # Single `ci_benchmarks` executable, 6 source files
├── runCfg/                 # IntelliJ run config XMLs (6 files)
└── script/                 # clean_work_space.py (removes cmake-build-* dirs)
```

## Build

```powershell
# Configure (prefers explicit CMAKE_TOOLCHAIN_FILE; falls back to $env:VCPKG_ROOT)
cmake -S . -B build

# Build all (default Debug; all targets)
cmake --build build

# Build specific target
cmake --build build --target server_app
cmake --build build --target client_app
cmake --build build --target ci_benchmarks
```

- **C++ standard**: C++26 (`CMAKE_CXX_STANDARD 26`)
- **Build type**: Default Debug (auto-set if none specified); supports Release, MinSizeRel, RelWithDebInfo
- **MSVC**: UTF-8 source/execution character set flags auto-added (`/utf-8` and friends)
- **Toolchain**: vcpkg via `$env:VCPKG_ROOT` or explicit `CMAKE_TOOLCHAIN_FILE`
- **Minimum CMake**: 3.30

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
20 directories → 20 `ut_<underscore_path>` executables:  
`aop`, `auth`, `buffer`, `cache`, `client`, `crypto`, `data_structure`, `filesystem`,  
`gen`, `interface`, `io`, `log`, `rpc`, `serialization`, `server`, `sql`, `system`,  
`thread`, `time`, `toolkit`

Each test exe compiles shared support sources (auth/config params from client/server) — expect slower first build. All tests runnable in parallel via `ctest -j N`.

Benchmark sources (6 files) in `ci/benchmark/src/`:
- `cache_benchmark.cc`, `crypto_benchmark.cc`, `data_structure_benchmark.cc`,
  `hash_benchmark.cc`, `tier2_benchmark.cc`, `toolkit_benchmark.cc`

## Proto / gRPC

Only one proto: `proto/src/RpcService.proto` (AuthService: 6 RPCs).  
Regenerate with `cmake --build build --target generate_proto`.  
Requires `grpc_cpp_plugin` from vcpkg (`$env:VCPKG_ROOT/installed/x64-windows/tools/grpc/`).  
Output goes to `proto/generated/` (4 files; gitignored; do not edit manually).

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
  the executable's working directory. `client/src/config/ConfigParam.h` uses `.h` suffix — the only
  non-`.hpp` header in the project.
- **glog**: All modules use glog. Both apps log to stderr with custom format. The `glog_service` lib
  wraps initialization (`GLogConfigurator`), prefix formatting (`PrefixFormatter`), and params (`GLogParam`).
- **data_structure module**: Largest module in `common/` with ~85 .hpp and ~30 .cc files across 20
  subdirectories. Covers graph algorithms (BFS, DFS, Dijkstra, KruskalMST, PrimMST, BellmanFord,
  TarjanSCC, FloydWarshall, TopologicalSort, CycleDetection, BipartiteCheck, BridgesArticulation),
  probabilistic data structures (BloomFilter, HyperLogLog, CountMinSketch, TDigest, MinHash),
  spatial structures (RTree, KDTree, IntervalTree), persistent data structures (PersistentVector),
  concurrent containers (MichaelScottQueue, TreiberStack, ConcurrentHashMap, SeqLock),
  consistent hashing (ConsistentHash, CuckooHashMap, RobinHoodHashMap),
  union-find with rollback (UnionSetWithRollback), Aho-Corasick string search,
  and tree structures across 8 sub-subdirectories (AVL, RedBlack, Splay, Treap, B-Tree, B+Tree,
  Trie, LSMTree, FenwickTree, SegmentTree, SparseTable, Quadtree, MerkleTree).

## vcpkg dependencies (14)

```
Boost (serialization), glog, gtest, yaml-cpp, RapidJSON, OpenSSL, DirectXMath,
Protobuf, gRPC, magic_enum, sqlite3, fmt, mysql-connector-cpp, benchmark
```

14 explicit `find_package` calls in root CMakeLists.txt, all managed through vcpkg.  
Toolchain: set `$env:VCPKG_ROOT` or pass `-DCMAKE_TOOLCHAIN_FILE=...` on configure.

## .gitignore highlights

| Pattern | Purpose |
|---|---|
| `build/`, `cmake-build*` | CMake build output directories |
| `proto/generated/` | Auto-generated gRPC stubs |
| `.idea` | JetBrains IDE config |
| `.cache/` | OpenCode artifact cache |
| `/.agentbridge/` | OpenCode agent bridge |
| `*.o`, `*.obj`, `*.lib`, `*.dll`, `*.exe` | Standard C/C++ build artifacts |

## OpenCode

This repo has OpenCode artifact caching (`.cache/cheshire/artifacts/`).  
Do not commit the `.cache/` directory (already in `.gitignore`).  
The `AGENTS.md` user instruction is read from this file.
