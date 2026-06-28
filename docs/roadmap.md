# cppforge — C++ Scaffold Framework 完整演进方案 v3.0

> 版本: v3.0 (已整合 5 Agent × 2 轮评审反馈)
> 生成日期: 2026-06-29
> 状态: 方案设计完毕（所有 Agent 条件已解决）
> 命名空间: cppforge::（脚手架全部模块统一前缀）

---

## 1. 项目定位

cppforge 是一个 **C++ 脚手架框架**，定位类似 Spring Boot 在 Java 生态：

- **可复用的基底**：任何 C++ 项目 `find_package(cppforge)` 即获得完整框架能力
- **模块化启动器**（starter）：认证、缓存、调度等开箱即用的功能组件
- **最小侵入**：提供接口但不强制继承特定基类

---

## 2. 当前资产清点

### 脚手架核心

| 目录 | CMake 目标 | 说明 |
|------|-----------|------|
| `common/` | `cppforge_common_pkg` | **核心库** — 17 模块，~300 源文件，192 .hpp + 106 .cc |
| `log/` | `cppforge_glog_service` | **日志门面** — 基于 glog 的 YAML 配置封装 |
| `script/` | — | **工具脚本** — `clean_work_space.py` |

### 示例项目（不属于脚手架，最终剥离为 examples/）

| 目录 | 当前用途 | 最终归属 |
|------|---------|---------|
| `proto/` | RpcService.proto + 代码生成 | 移入 `starters/auth/proto/` |
| `client/` | gRPC 认证客户端 | 降为 `examples/client_app/` 薄壳 |
| `server/` | gRPC 认证服务端 | 降为 `examples/server_app/` 薄壳 |
| `ci/` | 测试 + 基准 | 降为 `examples/tests/` + `examples/benchmarks/` |

---

## 3. 已知 P0 阻塞问题（执行前必须修复）

| # | 问题 | 文件 | 修复方案 |
|---|------|------|---------|
| P0.1 | `common_pkg` 链接 `glog_service` 形成循环依赖 | `common/CMakeLists.txt:37` | 切断 `common_pkg → glog_service` 链接；`glog_service` 改为 PUBLIC 依赖 `common_pkg` |
| P0.2 | `project()` 缺少 VERSION 参数 | `CMakeLists.txt:46` | 改为 `project(cppforge VERSION 0.1.0 LANGUAGES CXX)` |
| P0.3 | 所有 11 个依赖标记为 PRIVATE → 消费者无传递依赖 | `common/CMakeLists.txt:27-38` | 逐文件 grep 公开头文件的 `#include`，追溯外部库归属，仅提升必需的至 PUBLIC |
| P0.4 | server_app 链接 benchmark::benchmark_main | `server/CMakeLists.txt:41-42` | 删除 benchmark 和 benchmark_main |
| P0.5 | DirectXMath 阻塞非 Windows 构建 | `CMakeLists.txt:58` | 包裹 `if(WIN32)` |
| P0.6 | 硬编码 x64-windows vcpkg triplet | `proto/CMakeLists.txt:28` | 从 CMAKE_HOST_SYSTEM_PROCESSOR 动态检测 |
| P0.7 | log/ 通过 ../common/src 硬编码路径引用 common | `log/CMakeLists.txt:24` | 改为 target_link_libraries(PUBLIC common_pkg) |

### 其他硬编码 `../` 路径（P0.7 时一并修复）

| 文件 | 行 | 当前路径 | 修复方案 |
|------|-----|---------|---------|
| `client/CMakeLists.txt` | 22-27 | `../common/src`, `../proto`, `../log/src` | 改为 target_link_libraries 引用公共目标 |
| `server/CMakeLists.txt` | 22-27 | 同上 | 同上 |
| `ci/gtest/CMakeLists.txt` | 13-28 | `../../client/`, `../../server/` | 改为 target_link_libraries |
| `ci/benchmark/CMakeLists.txt` | 25-31 | 同上 | 同上 |

---

## 4. 目标目录结构

```
cplusplus_project_template/
├── CMakeLists.txt                    # project(cppforge VERSION 0.1.0)
├── vcpkg.json                        # vcpkg manifest 模式
├── cmake/                            # CMake 模板文件
│   └── cppforge-config.cmake.in
│
├── common/                           # ═══ 核心库 ═══
│   ├── CMakeLists.txt                # → cppforge::common_pkg
│   ├── include/
│   │   └── cppforge/                 # ★★★ 物理移动后的公共头文件
│   │       ├── Version.h             # 版本头（configure_file 生成）
│   │       ├── interface/            # 18 接口
│   │       ├── toolkit/              # 公共工具
│   │       ├── data_structure/       # 20 子目录
│   │       └── ...                   # 其他模块
│   └── src/                          # .cc 实现文件保留
│
├── log/                              # ═══ 日志门面 ═══
│   ├── CMakeLists.txt                # → cppforge::glog_service
│   ├── include/cppforge/glog/
│   │   ├── config/GLogConfigurator.hpp
│   │   ├── formatter/PrefixFormatter.hpp
│   │   └── param/GLogParam.hpp
│   └── src/
│
├── starters/                         # ═══ 功能启动器 ═══
│   ├── CMakeLists.txt
│   ├── auth/                         # 认证 starter
│   │   ├── CMakeLists.txt            # → cppforge::starter_auth
│   │   ├── proto/AuthService.proto
│   │   ├── include/cppforge/starter/auth/
│   │   │   ├── IAuthService.hpp
│   │   │   ├── IAuthHandler.hpp
│   │   │   ├── AuthConfig.hpp
│   │   │   ├── AuthServiceFactory.hpp
│   │   │   ├── server/AuthRpcServiceImpl.hpp
│   │   │   └── client/AuthRpcClient.hpp
│   │   └── src/
│   └── ... (未来更多 starter)
│
├── examples/                         # ═══ 使用示例 ═══
│   ├── CMakeLists.txt                # option(BUILD_EXAMPLES)
│   ├── sample_project/               # 最小外部消费者验证
│   ├── server_app/                   # 认证服务端薄壳
│   ├── client_app/                   # 认证客户端薄壳
│   ├── tests/                        # 集成测试
│   └── benchmarks/                   # 性能基准
│
├── script/
│   └── clean_work_space.py
│
└── docs/
    ├── roadmap.md                    # 本文件
    ├── starter-guide.md
    └── api/
```

---

## 5. 命名规范

脚手架全部 C++ 代码统一使用 `cppforge::` 根命名空间，下一级匹配模块目录名：

| 维度 | 规则 | 示例 |
|------|------|------|
| CMake 目标名 | `cppforge_<module>` | `cppforge_common_pkg` |
| CMake 命名空间 | `cppforge::` | `cppforge::common_pkg` |
| C++ 命名空间 | `cppforge::<dir>::` 匹配目录层级 | `cppforge::toolkit::StringToolkit` |
| C++ 命名空间（glog）| `cppforge::glog::<sub>` | `cppforge::glog::config::GLogConfigurator` |
| 头文件路径 | `<cppforge/<dir>/<File>.hpp>` | `#include <cppforge/toolkit/StringToolkit.hpp>` |
| 宏前缀 | `CPPFORGE_` | `CPPFORGE_VERSION` |
| CMake 包名 | `cppforge` | `find_package(cppforge CONFIG REQUIRED)` |
| Config 文件名 | `cppforge-config.cmake` | 全小写，符合 CMake 惯例 |

---

## 6. 分阶段演进路线

### 阶段验证门禁约定

每个阶段完成后必须满足以下条件才能进入下一阶段：
- **编译**：`cmake --build build` 全部 target 通过
- **测试**：`ctest --test-dir build -j 16 -R "^ut_" --extra-verbose` 全部通过
- **提交**：`git commit` 并打阶段标签（如 `phase-0-complete`）

---

### Phase 0: 前置修复 + 命名空间迁移（重构第一步）

**目标**：修复所有 P0 阻塞问题，完成 `common::*` + `glog::*` → `cppforge::*` 命名空间迁移

| 任务 | 细节 | 影响范围 |
|------|------|---------|
| 0.1 | 切断 `common_pkg → glog_service` 循环依赖 | `common/CMakeLists.txt` |
| 0.2 | `project(cppforge VERSION 0.1.0 LANGUAGES CXX)` | 根 `CMakeLists.txt` |
| 0.3 | 审计 PUBLIC/PRIVATE 依赖 — 逐文件 grep 公开头文件追溯依赖 | `common/CMakeLists.txt`, 全部 `.hpp` |
| 0.4 | 删除 `server_app` 中 `benchmark::benchmark_main` 链接 | `server/CMakeLists.txt` |
| 0.5 | DirectXMath 包裹 `if(WIN32)` | 根 `CMakeLists.txt` |
| 0.6 | 动态检测 vcpkg triplet | `proto/CMakeLists.txt` |
| 0.7 | 修复全部 5 处硬编码 `../` 路径（log + client + server + ci/gtest + ci/benchmark）| 5 个 CMakeLists.txt |
| 0.8 | **命名空间迁移**：`common::*` + `glog::*` → `cppforge::*`，分模块逐步进行：<br>interface(0) → toolkit → gen → time → buffer → io → filesystem → auth → rpc → serialization → sql → crypto → system → thread → cache → aop → data_structure(17) → glog_config → glog_formatter → glog_param(20) | ~350 源文件，~180 测试 |
| 0.9 | 创建 `vcpkg.json` manifest，锁定全部 14 个依赖 | 根目录 |
| 0.10 | **阶段验证**：全部 target 编译 + 全部测试通过 + `git tag phase-0-complete` | — |

**命名空间迁移执行策略**：
1. 每模块完成后立即编译当前模块的 `ut_<模块>_test` 验证
2. 使用 sed/脚本批量替换，人工仅审核边界情况
3. 在 feature branch 上操作，master 保持稳定

---

### Phase 1: Include 结构重塑

**目标**：头文件从 `src/` 物理移动到 `include/cppforge/`

| 任务 | 细节 | 影响范围 |
|------|------|---------|
| 1.1 | 在 `common/` 创建 `include/cppforge/` 目录树 | 新建目录 |
| 1.2 | 所有 `.hpp` 从 `common/src/` 移至 `common/include/cppforge/` | ~192 .hpp 文件 |
| 1.3 | 更新所有 `#include` 路径（相对 → `<cppforge/...>`） | 全部 .cc/.hpp |
| 1.4 | 更新 CMakeLists.txt 包含路径 | 全部 CMakeLists |
| 1.5 | 同样处理 `log/`（`src/` → `include/cppforge/glog/`） | log/ 目录 |
| 1.6 | **阶段验证**：全部编译 + 全部测试通过 + `git tag phase-1-complete` | — |

---

### Phase 2: CMake Install/Export

**目标**：`find_package(cppforge CONFIG)` + `target_link_libraries(myapp PRIVATE cppforge::common_pkg)` 可用

| 任务 | 细节 |
|------|------|
| 2.1 | 创建 `cmake/cppforge-config.cmake.in` 模板（含 DirectXMath WIN32 守卫 + MSVC UTF-8 传播 + C++ standard 传播） |
| 2.2 | common 的 `$<BUILD_INTERFACE>`/`$<INSTALL_INTERFACE>` 双模式 |
| 2.3 | log 的双模式路径 |
| 2.4 | `install(TARGETS ... EXPORT ...)` 规则 |
| 2.5 | `install(DIRECTORY include/ ...)` 头文件安装 |
| 2.6 | `write_basic_package_version_file()` |
| 2.7 | `install(EXPORT ... NAMESPACE cppforge::)` |
| 2.8 | 添加 `option(BUILD_EXAMPLES OFF)` |
| 2.9 | 创建 `examples/sample_project/` 外部消费者验证 |
| 2.10 | 创建 `common/include/cppforge/Version.h.in` + `configure_file()` 生成版本头 |
| 2.11 | **阶段验证**：全部编译 + 测试 + 外部 sample_project 编译 + `git tag phase-2-complete` |

---

### Phase 3: Starter 架构（认证提取）

**目标**：认证功能从 4 个分散目录提取为独立 `starter/auth/`

**文件迁移映射**：

| 源文件 | 目标文件 | 操作 |
|--------|---------|------|
| `proto/src/RpcService.proto` | `starters/auth/proto/AuthService.proto` | 移动 + 重命名 |
| `server/src/auth/AuthRpcService.*` | `starters/auth/src/server/AuthRpcServiceImpl.*` | 移动 + 重命名 + namespace |
| `client/src/auth/AuthRpcService.*` | `starters/auth/src/client/AuthRpcClient.*` | 移动 + 重命名 + namespace |
| `server/src/auth/AuthRpcParam.*` | `starters/auth/src/config/AuthConfig.*` | 合并（含 YAML::convert 特化） |
| `client/src/auth/AuthRpcParam.*` | 同上（合并入 AuthConfig） | 合并 |
| `ci/gtest/src/server/auth/AuthRpcParamTest.cc` | `starters/auth/test/AuthConfigTest.cc` | 移动 + 合并 |
| `ci/gtest/src/client/AuthRpcParamTest.cc` | 同上（合并入 AuthConfigTest） | 合并 |

| 任务 | 细节 |
|------|------|
| 3.1 | 创建 `starters/` + `starters/auth/` 目录结构 |
| 3.2 | 移入 proto 文件 + 重建生成规则 |
| 3.3 | 合并 `AuthRpcParam` 为统一 `AuthConfig`（含全部 7 个参数，服务端字段默认值，参见设计取舍） |
| 3.4 | server `AuthRpcService` → `AuthRpcServiceImpl`（namespace: `cppforge::starter::auth`） |
| 3.5 | client `AuthRpcService` → `AuthRpcClient`（namespace: `cppforge::starter::auth`） |
| 3.6 | 创建 `AuthServiceFactory` 统一工厂（`createServer()` / `createClient()` / `registerServices()`） |
| 3.7 | starters 的 CMake Install/Export 规则 |
| 3.8 | 示例应用瘦身（`main()` 缩减至 ~10 行） |
| 3.9 | 测试迁移 + 共享源码清理 |
| 3.10 | **阶段验证**：全部编译 + 全部测试 + auth starter 单元测试 + `git tag phase-3-complete` |

---

### Phase 4: DI / IoC 容器

**目标**：ServiceCollection → ServiceProvider → ApplicationContext

| 任务 | 细节 |
|------|------|
| 4.1 | 类型擦除容器（`std::type_index` → `std::any` 映射） |
| 4.2 | ServiceCollection 注册 API（Singleton / Scoped / Transient） |
| 4.3 | ServiceProvider 构造函数参数自动解析 |
| 4.4 | ApplicationContext 生命周期（run() / shutdown()） |
| 4.5 | Starter 适配：各 starter 提供 `registerServices()` |
| 4.6 | **阶段验证**：全部编译 + 全部测试 + DI 容器单元测试 |

---

### Phase 5: 中间件 + 可观测性

| 任务 | 细节 |
|------|------|
| 5.1 | HTTP 接口抽象（IHttpServer, IHttpRequest, IHttpResponse） |
| 5.2 | 中间件链（MiddlewareChain + 拦截器） |
| 5.3 | 路由注册（Builder 模式） |
| 5.4 | 指标门面 + Prometheus 导出 |
| 5.5 | 健康检查（`/health` 端点） |
| 5.6 | 事件总线（publish/subscribe） |
| 5.7 | 配置热重载（FileWatcher + 回调） |
| 5.8 | **阶段验证**：全部编译 + 全部测试 + 中间件链集成测试 |

---

### Phase 6: 更多 Starters + CI

| 任务 | 细节 |
|------|------|
| 6.1 | `starter_cache` — 多级缓存（本地 LRU/LFU + Redis） |
| 6.2 | `starter_scheduler` — 任务调度（Cron + 延时 + 周期） |
| 6.3 | `starter_security` — RBAC + JWT |
| 6.4 | `starter_tracing` — OpenTelemetry 集成 |
| 6.5 | GitHub Actions CI（matrix build + install-verify job） |
| 6.6 | Doxygen API 文档生成 |
| 6.7 | **最终验证**：全量编译 + 全量测试 + 跨平台 CI 绿色 + `git tag v1.0.0` |

---

## 7. 依赖关系图（修复后）

```
外部消费者项目
    │
    └── find_package(cppforge CONFIG REQUIRED)
            │
    ┌───────┴───────────────────────┐
    │                                │
cppforge::common_pkg                cppforge::glog_service
    │  PUBLIC                          │  PUBLIC
    ├── Boost::headers                ├── glog::glog
    ├── yaml-cpp::yaml                ├── yaml-cpp::yaml
    ├── rapidjson                     ├── fmt::fmt
    ├── OpenSSL::SSL                  └── cppforge::common_pkg (PUBLIC)
    ├── magic_enum
    ├── DirectXMath (WIN32 only)
    ├── sqlite3
    ├── fmt::fmt
    └── glog::glog
            │
    ┌───────┘
    │
cppforge::starter_auth
    ├── cppforge::common_pkg (PUBLIC)
    ├── cppforge::glog_service (PUBLIC)
    ├── gRPC::grpc++
    └── Protobuf::libprotobuf
```

循环依赖已解除：`common_pkg → glog_service` 的连接已切断。
`glog_service → common_pkg` 是唯一的依赖方向（PUBLIC）。

---

## 8. CMake Install/Export 核心设计

### Config 文件安装结构

```
install_prefix/
└── lib/cmake/cppforge/
    ├── cppforge-config.cmake         # 统一入口
    ├── cppforge-config-version.cmake # 版本比较
    ├── cppforgeTargets.cmake         # target 定义
    └── cppforgeTargets-release.cmake # release 配置
```

### cmake/cppforge-config.cmake.in 模板

```cmake
@PACKAGE_INIT@

# 重新查找传递依赖
include(CMakeFindDependencyMacro)
find_dependency(Boost CONFIG COMPONENTS serialization)
find_dependency(yaml-cpp CONFIG REQUIRED)
find_dependency(RapidJSON CONFIG REQUIRED)
find_dependency(OpenSSL REQUIRED)
find_dependency(magic_enum CONFIG REQUIRED)
find_dependency(unofficial-sqlite3 CONFIG REQUIRED)
find_dependency(fmt CONFIG REQUIRED)
find_dependency(glog CONFIG REQUIRED)

# DirectXMath 仅限 Windows
if(WIN32)
    find_dependency(directxmath CONFIG)
endif()

# 加载 targets
include("${CMAKE_CURRENT_LIST_DIR}/cppforgeTargets.cmake")

# Windows UTF-8 传播
if(MSVC)
    add_compile_options("/utf-8")
endif()

# C++ 标准传播（消费者无需手动 set CMAKE_CXX_STANDARD）
if(NOT MSVC)
    target_compile_features(cppforge_common_pkg INTERFACE cxx_std_26)
endif()

check_required_components(cppforge_common_pkg cppforge_glog_service)
```

### BUILD_INTERFACE / INSTALL_INTERFACE 双模式

```cmake
# 构建时：-I/path/to/common/include
# 安装后：-I/install_prefix/include
target_include_directories(cppforge_common_pkg PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
```

### CMake 目标名设计

- 内部构建目标名：`cppforge_common_pkg`（前缀 `cppforge_` 避免与外部项目重名）
- 导出后命名空间：`cppforge::common_pkg`
- 内部消费者通过 `cppforge_common_pkg` 引用；外部消费者通过 `cppforge::common_pkg`

---

## 9. 技术债清单

| 债务 | 严重度 | 处理时机 |
|------|--------|---------|
| `file(GLOB_RECURSE)` 7 处使用 | P1 | Phase 2 后考虑显式文件列表 |
| 硬编码 `../` 路径 5 处 | P1 | Phase 0.7 修复 |
| Proto include 路径脆弱 | P1 | Phase 3.2 |
| Benchmark 混入 CTest 测试集 | P2 | Phase 6 |
| `ci/` 测试共享源码导致全量重编译 | P1 | Phase 3.9 |
| ConfigParam 硬编码 YAML 路径 | P1 | Phase 3 |
| 缺少 Doxygen 配置 | P2 | Phase 6 |
| 测试覆盖率报告 | P2 | Phase 6 |

---

## 10. 关键取舍记录

| 决策点 | 选择 | 备选 | 理由 |
|--------|------|------|------|
| 静态 vs 动态库 | 静态 | 动态 | 避免 Windows DLL 问题，简化分发 |
| 头文件组织 | 物理移动到 `include/cppforge/` | install 映射 | 目录结构清晰 vs 改动量大的取舍 |
| 命名空间迁移时机 | Phase 0 第一步 | 后续再做 | 一次性完成，避免重复破坏 |
| 命名空间模式 | `cppforge::<dir>::` | 单层 `cppforge::` | 匹配目录层级，避免命名冲突 |
| CMake 目标重命名 | Phase 0 或 Phase 2 | 不重命名 | 与包名一致，便于外部识别 |
| 版本号策略 | `SameMajorVersion` | `AnyNewerVersion` | 主版本内兼容，保守策略 |
| CI 时机 | Phase 6（最后） | Phase 1 | 先完成全部功能重构 |
| `GLOB_RECURSE` | 保留（暂不替换） | 显式列表 | 当前可工作，稳定后再改 |
| starters vs 插件 | starters | dlopen 插件 | C++ 无标准插件机制 |
| gRPC vs HTTP | 双协议栈（gRPC 先） | 仅 HTTP | 已有 gRPC 基础 |

---

## 11. 外部消费者体验

```bash
# 1. 安装 cppforge
git clone https://github.com/your/cppforge.git
cd cppforge
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix ~/cppforge
```

```cmake
# my_project/CMakeLists.txt
cmake_minimum_required(VERSION 3.30)
project(my_app)
find_package(cppforge CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE
    cppforge::common_pkg
    cppforge::glog_service
)
```

```cpp
// my_project/main.cpp
#include <cppforge/Version.h>
#include <cppforge/toolkit/StringToolkit.hpp>
#include <cppforge/glog/config/GLogConfigurator.hpp>
#include <iostream>

int main() {
    std::cout << "cppforge v" << CPPFORGE_VERSION << std::endl;

    // 使用 StringToolkit
    auto parts = cppforge::toolkit::StringToolkit::split(
        "hello world", " ");
    for (const auto& p : parts)
        std::cout << "  [" << p << "]" << std::endl;

    // 使用 glog 日志门面（NVI 模式：public config() → protected doConfig()）
    GLogConfigurator configurator("path/to/glog_config.yml");
    configurator.config();
    LOG(INFO) << "App started!";

    return 0;
}
```

```bash
# 编译运行
cmake -S . -B build -DCMAKE_PREFIX_PATH=~/cppforge
cmake --build build && ./build/my_app
```

---

## 附录 A: Agent 评审历程

| 轮次 | Agent | 裁决 | 条件数 |
|------|-------|------|--------|
| 1 | Architect | 有条件通过 | 新增 5 条 |
| 1 | Tech-Lead | 有条件通过 | 新增 2 条 |
| 1 | Product-Manager | 有条件通过 | 新增 3 条 |
| 1 | Senior-Fullstack | 有条件通过 | 新增 8 条 |
| 1 | DevOps | 有条件通过 | 新增 3 条 |
| 2 | 全部 5 Agent | 二次有条件通过 | 共 ~21 条，去重后 12 条 |
| — | **已解决** | **v3.0 全部采纳** | — |

### 已解决的 12 条条件汇总

| # | 条件 | 解决方式 |
|---|------|---------|
| C1 | 添加 vcpkg.json 创建任务 | Phase 0.9 |
| C2 | 添加 Version.h 生成任务 | Phase 2.10 |
| C3 | 添加阶段结束验证门禁 | 每个 Phase 尾部署 + Section 6 开头的门禁约定 |
| C4 | 修复消费者示例 API（GLogConfigurator, NVI） | Section 11 已修正 |
| C5 | 修复 DirectXMath find_dependency 守卫 | Section 8 模板已加 `if(WIN32)` |
| C6 | 文档化 C++ 标准传播 | Section 8 模板已加 `target_compile_features` |
| C7 | 展开 P0.3 审计方法 | Section 3 已加 grep 流程说明 |
| C8 | 展开 P0.8 模块顺序 | 20 个模块逐一列出的逐步顺序 |
| C9 | 命名空间迁移覆盖 glog:: | Phase 0.8 已扩展 + Section 5 |
| C10 | 文件级迁移映射表（Phase 3）| Section 6 Phase 3 已加 git mv 映射表 |
| C11 | 全部硬编码 ../ 路径修复 | 5 处路径全部列在 Section 3 |
| C12 | CMake 目标重命名策略 | Section 8 已文档化 |