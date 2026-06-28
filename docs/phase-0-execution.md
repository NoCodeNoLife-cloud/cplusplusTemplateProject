# cppforge Phase 0 Execution Manual

> Version 1.0 -- Step-by-step guide for the first refactoring phase
> Corresponds to docs/roadmap.md Section 6 Phase 0

## Prerequisites

- Python 3.10+ (for migration scripts)
- Git (feature branch: refactor/phase-0)
- CMake 3.30+
- Visual Studio 2022 (MSVC v143)
- vcpkg with VCPKG_ROOT set

## Branch Strategy

Work on a feature branch, commit after each subtask:
  git checkout -b refactor/phase-0
  [do task]
  git commit -m "refactor: P0.X description"

## Task 0.1: Fix Circular Dependency

File: common/CMakeLists.txt (line 37)
Before: target_link_libraries(common_pkg PRIVATE ... glog_service ...)
After:  target_link_libraries(common_pkg PRIVATE ... [REMOVE glog_service] ...)
Verify: cmake -S . -B build && cmake --build build
Rollback: git checkout -- common/CMakeLists.txt

## Task 0.2: Add Project Version

File: CMakeLists.txt root (line 46)
Before: project()
After:  project( VERSION 0.1.0 DESCRIPTION "cppforge" LANGUAGES CXX)
Verify: cmake -S . -B build (check output for version)

## Task 0.3: PUBLIC/PRIVATE Dependency Audit

Method: grep all public .hpp for third-party #includes
Promote to PUBLIC: Boost, yaml-cpp, rapidjson, OpenSSL, magic_enum, fmt, glog
Keep PRIVATE: DirectXMath, sqlite3, mysql-connector-cpp

## Task 0.4: Remove Benchmark from server_app

File: server/CMakeLists.txt (lines 41-42)
Remove these two lines:
  benchmark::benchmark
  benchmark::benchmark_main

## Task 0.5: Guard DirectXMath

File: CMakeLists.txt root (line 58)
Before: find_package(directxmath CONFIG REQUIRED)
After:  if(WIN32) find_package(directxmath CONFIG REQUIRED) endif()

## Task 0.6: Dynamic vcpkg Triplet

File: proto/CMakeLists.txt (line 28)
Replace hardcoded x64-windows with:
  if(WIN32): set triplet_suffix windows
  elseif(APPLE): set triplet_suffix osx
  else(): set triplet_suffix linux
  Detect arch from CMAKE_HOST_SYSTEM_PROCESSOR

## Task 0.7: Fix All Hardcoded ../ Paths

5 files need fixing:
  (1) log/CMakeLists.txt: ../common/src -> PUBLIC common_pkg link
  (2) client/CMakeLists.txt: ../common/src, ../proto, ../log/src -> remove, use target_link
  (3) server/CMakeLists.txt: same as client
  (4) ci/gtest/CMakeLists.txt: ../../client, ../../server -> target_link
  (5) ci/benchmark/CMakeLists.txt: same as ci/gtest

## Task 0.8: Namespace Migration

20 modules in order:
  1. interface (ut_interface)
  2. toolkit (ut_toolkit)
  3. gen (ut_gen)
  4. time (ut_time)
  5. buffer (ut_buffer)
  6. io (ut_io)
  7. filesystem (ut_filesystem)
  8. auth (ut_auth)
  9. rpc (ut_rpc)
  10. serialization (ut_serialization)
  11. sql (ut_sql)
  12. crypto (ut_crypto)
  13. system (ut_system)
  14. thread (ut_thread)
  15. cache (ut_cache)
  16. aop (ut_aop)
  17. data_structure (ut_data_structure)
  18. glog config (ut_log)
  19. glog formatter (ut_log)
  20. glog param (ut_log)

For each module: run sed/script to replace namespace, then compile+test.

## Task 0.9: Create vcpkg.json

Create root-level vcpkg.json with all 14 dependencies.
See docs/roadmap.md Section 3 for the full list.

## Task 0.10: Phase Verification

Checklist:
  [ ] cmake -S . -B build succeeds
  [ ] cmake --build build succeeds
  [ ] ctest --test-dir build -j 16 -R "^ut_" --extra-verbose all pass
  [ ] server_app has no benchmark symbols
  [ ] git add -A && git commit
  [ ] git tag phase-0-complete

---
End of Phase 0 Execution Manual