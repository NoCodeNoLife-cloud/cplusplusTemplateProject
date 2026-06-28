# cppforge 编码规范 v1.0

> **适用版本**: C++26 (CMAKE_CXX_STANDARD 26)  
> **生效日期**: 2026-06-29  
> **审核人**: Tech-Lead / Security-Engineer  
> **对应文件**: .clang-format, AGENTS.md, docs/roadmap.md

本文档是 cppforge 框架的官方编码规范。所有提交到本仓库的代码 **必须** 遵守以下规则。  
每条规则均附 **正面示例 (Good)** 与 **反面示例 (Bad)**，反面示例标记 ❌。

---

## 目录

1. [命名空间 (Namespace) 规范](#1-命名空间-namespace-规范)
2. [头文件规范](#2-头文件规范)
3. [文档注释 (Documentation Comments)](#3-文档注释-documentation-comments)
4. [命名规范 (Naming Conventions)](#4-命名规范-naming-conventions)
5. [NVI (Non-Virtual Interface) 模式](#5-nvi-non-virtual-interface-模式)
6. [单例 (Singleton) 模式](#6-单例-singleton-模式)
7. [工厂 (Factory) 模式](#7-工厂-factory-模式)
8. [错误处理 (Error Handling)](#8-错误处理-error-handling)
9. [线程安全 (Thread Safety)](#9-线程安全-thread-safety)
10. [内存管理 (Memory Management)](#10-内存管理-memory-management)
11. [C++ 标准与语言特性](#11-c-标准与语言特性)
12. [包含保护 (Include Guard)](#12-包含保护-include-guard)
13. [格式化 (Formatting)](#13-格式化-formatting)
14. [CMake 构建规范](#14-cmake-构建规范)
15. [附录：快速检查清单](#15-附录快速检查清单)

---
## 1. 命名空间 (Namespace) 规范

### 规则

所有 cppforge 代码使用 cppforge:: 作为根命名空间，下一级名称 **必须** 与源代码目录路径相匹配。

| 模块路径 | 命名空间 |
|---|---|
| common/include/cppforge/toolkit/ | cppforge::toolkit |
| common/include/cppforge/interface/io/ | cppforge::interface::io |
| common/include/cppforge/interface/serialization/ | cppforge::interface::serialization |
| common/include/cppforge/interface/task/ | cppforge::interface::task |
| common/include/cppforge/crypto/hash/ | cppforge::crypto::hash |
| log/include/cppforge/glog/config/ | cppforge::glog::config |
| starters/auth/include/cppforge/starter/auth/ | cppforge::starter::auth |

### 规则细节

- 命名空间层级 **必须** 与物理目录层级一一对应。
- 不允许使用单层 cppforge:: 来包含不同模块的类——这会丢失目录信息的可追溯性。

`cpp
// Good: 命名空间匹配目录层级
// 文件路径: common/include/cppforge/toolkit/StringToolkit.hpp
namespace cppforge::toolkit
{
    class StringToolkit { /* ... */ };
}

// Good: 多级命名空间
// 文件路径: common/include/cppforge/interface/io/IReadable.hpp
namespace cppforge::interface::io
{
    class IReadable { /* ... */ };
}
`

`cpp
// Bad: 命名空间与实际路径不匹配
namespace cppforge::string_utils  // 应该用 cppforge::toolkit
{
    class StringToolkit { /* ... */ };
}

// Bad: 使用单层命名空间丢失模块信息
namespace cppforge
{
    class StringToolkit { /* ... */ };  // 这是 toolkit 还是什么？
}

// Bad: 旧命名空间（尚未迁移的代码）
namespace common::toolkit  // 应该迁移到 cppforge::toolkit
{
    // ...
}
`

---

## 2. Headers

All headers MUST use #pragma once.

Include order (4 groups, blank line between groups):
1. Own header (.cc only)
2. C++ standard library
3. Third-party (vcpkg)
4. cppforge internal

Use angle brackets with cppforge/ prefix.
No ../ relative paths.

---


## 3. Documentation Comments

Use /// @brief style (Doxygen-compatible):

`cpp
/// @brief Brief description (one line)
/// @details Detailed explanation if needed
/// @param name Description of parameter
/// @return Description of return value
/// @throws std::runtime_error When X happens
`

Every public method MUST have /// @brief. Internal helpers SHOULD.

---

## 4. Naming Conventions

| Category | Convention | Example |
|----------|-----------|---------|
| Classes / Types | PascalCase | StringToolkit, AuthConfig |
| Interfaces | PascalCase with I prefix | IReadable, IAuthService |
| Functions / Methods | snake_case | getInstance(), doConfig() |
| Member variables | snake_case with trailing _ | name_, age_ |
| Constants / Macros | UPPER_SNAKE | CPPFORGE_VERSION |
| Namespaces | snake_case | cppforge::toolkit |
| Template params | PascalCase (T prefix) | TKey, TValue |

---

## 5. NVI (Non-Virtual Interface) Pattern

Public methods are non-virtual wrappers calling protected virtual doXxx():

`cpp
class IConfigurable {
public:
    void config() {         // Public NVI wrapper
        // preconditions
        doConfig();
        // postconditions
    }
protected:
    virtual void doConfig() = 0;  // override point
};
`

---

## 6. Singleton Pattern

Use Meyers' singleton (function-local static):

`cpp
class ConfigParam {
public:
    static ConfigParam& getInstance() {
        static ConfigParam instance;
        return instance;
    }
    // ...
};
`

Thread-safe in C++11+. Do NOT add manual mutex guards.

---

## 7. Factory Pattern

Use ObjectFactory<T> for generic factory needs.
Use typed XxxFactory static class for starter-specific factories.

---

## 8. Error Handling

- Throw std::runtime_error for recoverable runtime errors
- Throw std::invalid_argument for invalid parameters
- Catch-all in main(): StackTraceExceptionHandler::logException()
- Never swallow exceptions silently
- Log before throwing: LOG(ERROR) << msg; throw std::runtime_error(msg)

---

## 9. Thread Safety

Default: NOT thread-safe unless documented with @par Thread Safety.

---

## 10. Memory Management

No raw new/delete. Use std::unique_ptr, std::shared_ptr, std::make_unique.

---

## 11. C++ Standard

C++26 features allowed. Prefer readability over cleverness.
Allowed: ranges, concepts, coroutines.
Avoid: SFINAE (use concepts instead), template metaprogramming tricks.

---

## 12. Formatting

Enforced by .clang-format (Google-based):
- 4-space indent (not 2)
- ColumnLimit: 600 (no wrapping for most lines)
- PointerAlignment: Left (int* p not int *p)
- BreakBeforeBraces: Attach
- NamespaceIndentation: None

---

## 13. CMake Conventions

Target name: cppforge_<module> (build), cppforge::<module> (export).
Dependency type: PUBLIC if header exposes it, PRIVATE if .cc only.

---

## 14. Quick Checklist

- [ ] #pragma once only
- [ ] /// @brief on all public methods
- [ ] Namespace matches directory path
- [ ] No ../ in includes
- [ ] NVI pattern for configuration
- [ ] [[nodiscard]] on returning functions
- [ ] LOG(ERROR) before throwing
- [ ] Thread safety documented via @par Thread Safety
- [ ] No raw new/delete
- [ ] Smart pointers for ownership
