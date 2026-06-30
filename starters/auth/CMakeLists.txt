# ══════════════════════════════════════════════════════════════════════════
#  Starter: Auth — starter_auth
# ══════════════════════════════════════════════════════════════════════════

set(STARTER_AUTH starter_auth)

# ── gRPC plugin discovery ────────────────────────────────────────────────
find_program(gRPC_CPP_PLUGIN_EXECUTABLE
    NAMES grpc_cpp_plugin
    PATHS
        "$ENV{VCPKG_ROOT}/installed/$ENV{VCPKG_TARGET_TRIPLET}/tools/grpc"
        "$ENV{VCPKG_ROOT}/installed/x64-windows/tools/grpc"
        "$ENV{ProgramFiles}/gRPC/bin"
        "/usr/local/bin"
    DOC "Path to gRPC C++ plugin"
)

if (NOT gRPC_CPP_PLUGIN_EXECUTABLE)
    message(FATAL_ERROR "Could not find gRPC C++ plugin (grpc_cpp_plugin)")
endif()

# ── Proto generation ─────────────────────────────────────────────────────
set(AUTH_PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(AUTH_PROTO_FILE ${AUTH_PROTO_DIR}/AuthService.proto)
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

get_filename_component(AUTH_PROTO_NAME ${AUTH_PROTO_FILE} NAME_WE)
set(AUTH_PB_CC ${PROTO_GEN_DIR}/${AUTH_PROTO_NAME}.pb.cc)
set(AUTH_PB_H ${PROTO_GEN_DIR}/${AUTH_PROTO_NAME}.pb.h)
set(AUTH_GRPC_CC ${PROTO_GEN_DIR}/${AUTH_PROTO_NAME}.grpc.pb.cc)
set(AUTH_GRPC_H ${PROTO_GEN_DIR}/${AUTH_PROTO_NAME}.grpc.pb.h)

file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

add_custom_command(
    OUTPUT ${AUTH_PB_CC} ${AUTH_PB_H} ${AUTH_GRPC_CC} ${AUTH_GRPC_H}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS
        --cpp_out=${PROTO_GEN_DIR}
        --grpc_out=${PROTO_GEN_DIR}
        --plugin=protoc-gen-grpc=${gRPC_CPP_PLUGIN_EXECUTABLE}
        -I${AUTH_PROTO_DIR}
        ${AUTH_PROTO_FILE}
    DEPENDS ${AUTH_PROTO_FILE}
    COMMENT "Generating gRPC code from AuthService.proto"
)

# ── Sources ──────────────────────────────────────────────────────────────
file(GLOB_RECURSE AUTH_SRC_FILES src/*.cc)

# ── Library ──────────────────────────────────────────────────────────────
add_library(${STARTER_AUTH} STATIC
    ${AUTH_SRC_FILES}
    ${AUTH_PB_CC} ${AUTH_GRPC_CC}
)

target_include_directories(${STARTER_AUTH} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROTO_GEN_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${STARTER_AUTH} PUBLIC
    common_pkg
    glog_service
)

target_link_libraries(${STARTER_AUTH} PRIVATE
    protobuf::libprotobuf
    gRPC::grpc++
    glog::glog
    yaml-cpp::yaml-cpp
    fmt::fmt
)

# ── Install ──────────────────────────────────────────────────────────────
install(TARGETS ${STARTER_AUTH}
    EXPORT cppforgeTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/cppforge
    DESTINATION include
)