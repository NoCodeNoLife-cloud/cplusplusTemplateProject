# cppforge Build System Reference

> Version 1.0 -- Phase 2+ reference

## 1. Target Architecture

Core targets (always built):
  cppforge_common_pkg  -> cppforge::common_pkg   (core library)
  cppforge_glog_service -> cppforge::glog_service (logging facade)

Starter targets (optional, BUILD_STARTERS):
  cppforge_starter_<name> -> cppforge::starter_<name>

Example targets (optional, BUILD_EXAMPLES):
  server_app, client_app, ut_*, ci_benchmarks

## 2. Dependency Classification

PUBLIC (propagates to consumers):
  Boost::headers, yaml-cpp::yaml-cpp, rapidjson,
  OpenSSL::SSL, OpenSSL::Crypto, magic_enum::magic_enum,
  fmt::fmt, glog::glog

PRIVATE (implementation detail only):
  Microsoft::DirectXMath (if WIN32),
  unofficial::sqlite3::sqlite3,
  unofficial::mysql-connector-cpp::connector

## 3. Dual-Mode Include Paths

BUILD_INTERFACE: points to source tree include/
  $<BUILD_INTERFACE:/include>

INSTALL_INTERFACE: points to installed include/
  $<INSTALL_INTERFACE:include>

## 4. Config File Generation

Template: cmake/cppforge-config.cmake.in
Generated at install time to: lib/cmake/cppforge/
  cppforge-config.cmake (entry point)
  cppforge-config-version.cmake (version check)
  cppforgeTargets.cmake (target definitions)
  cppforgeTargets-release.cmake (release config)

## 5. C++ Standard Propagation

Consumers get C++26 automatically via:
  target_compile_features(cppforge_common_pkg INTERFACE cxx_std_26)

## 6. Build Options

  BUILD_EXAMPLES=OFF  (default)
  BUILD_STARTERS=ON   (default)
  BUILD_TESTING=ON   (default, Phase 6+)

## 7. Quick Commands

  cmake -S . -B build
  cmake --build build
  ctest --test-dir build -j 16 -R "^ut_" --extra-verbose
  cmake --install build --prefix install_dir

---
End of Build System Reference