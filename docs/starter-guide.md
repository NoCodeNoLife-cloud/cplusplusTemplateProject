# cppforge Starter Development Guide

> Version 1.0 - Phase 3+ reference

## 1. Overview

A starter is a self-contained, pluggable feature module.
Each starter has its own interfaces, implementation, config, proto definition (optional), and tests.

## 2. Directory Structure

starters/<name>/
  CMakeLists.txt
  proto/<Name>Service.proto             (optional, gRPC)
  include/cppforge/starter/<name>/
    I<Name>Service.hpp                  (public interface)
    <Name>Config.hpp                    (YAML config)
    <Name>Factory.hpp                   (unified factory)
    server/<Name>RpcServiceImpl.hpp     (server impl)
    client/<Name>RpcClient.hpp          (client stub)
  src/
    server/<Name>RpcServiceImpl.cc
    client/<Name>RpcClient.cc
    <Name>Config.cc
    <Name>Factory.cc
  test/
    CMakeLists.txt
    <Name>ConfigTest.cc
    <Name>FactoryTest.cc

## 3. Naming Rules

| Dimension | Rule | Example |
  C++ namespace | cppforge::starter::<name> | cppforge::starter::auth |
  CMake target | cppforge_starter_<name> | cppforge_starter_auth |
  Export name | cppforge::starter_<name> | cppforge::starter_auth |
  Include path | <cppforge/starter/<name>/...> | <cppforge/starter/auth/IAuthService.hpp> |

## 4. Factory Pattern

Each starter MUST provide a unified Factory class:
  static unique_ptr<I<Name>Service> createServer(const <Name>Config&)
  static unique_ptr<I<Name>Service> createClient(const <Name>Config&)
  static const string& getVersion()

The Factory is the ONLY entry point for consumers.

## 5. Interface Design

Follow NVI pattern:
  public: void start() -> calls protected: virtual void doStart()
  public: void shutdown() -> calls protected: virtual void doShutdown()

Document thread safety: @par Thread Safety in file header.
Mark all returning functions with [[nodiscard]].

## 6. Dependencies

PUBLIC (always): cppforge_common_pkg, cppforge_glog_service
PRIVATE (as needed): gRPC::grpc++, Protobuf::libprotobuf, etc.

## 7. CMakeLists.txt Template

set(TARGET cppforge_starter_<name>)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cc)
add_library( STATIC )
add_library(cppforge::starter_<name> ALIAS )
target_include_directories( PUBLIC
  $<BUILD_INTERFACE:/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries( PUBLIC
  cppforge_common_pkg
  cppforge_glog_service)

See docs/build-system.md for full CMake reference.
