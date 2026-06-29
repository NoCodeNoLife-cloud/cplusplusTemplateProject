# ══════════════════════════════════════════════════════════════════════════
#  Proto / gRPC code generation — grpc_service
# ══════════════════════════════════════════════════════════════════════════
#
# Generates C++ source and header files from .proto definitions, then
# packages them into a static library (grpc_service) consumed by both
# the client and server executables.
#
# ══════════════════════════════════════════════════════════════════════════

# ── Code-generation target ───────────────────────────────────────────────
set(PROTOBUF_PROJECT generate_proto)

# ── Proto source layout ──────────────────────────────────────────────────
set(PROTO_SOURCE_DIR "${PROJECT_SOURCE_DIR}/proto/src"
    CACHE PATH "Root directory for .proto files")
set(PROTO_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/proto/generated"
    CACHE PATH "Output directory for generated code")
file(GLOB_RECURSE PROTO_FILES "${PROTO_SOURCE_DIR}/*.proto")

set(PROTO_SRCS "")
set(PROTO_HDRS "")

# ── gRPC plugin discovery ────────────────────────────────────────────────
find_program(gRPC_CPP_PLUGIN_EXECUTABLE
    NAMES grpc_cpp_plugin
    PATHS
        "$ENV{VCPKG_ROOT}/installed/$ENV{VCPKG_TARGET_TRIPLET}/tools/grpc"
        "$ENV{VCPKG_ROOT}/installed/x64-windows/tools/grpc"
        "$ENV{ProgramFiles}/gRPC/bin"
        "/usr/local/bin"
    DOC "Path to gRPC C++ plugin (grpc_cpp_plugin)"
)

if (NOT gRPC_CPP_PLUGIN_EXECUTABLE)
    message(FATAL_ERROR "Could not find gRPC C++ plugin (grpc_cpp_plugin). "
            "Please verify vcpkg installation or try reinstalling gRPC.")
endif ()

message(STATUS "gRPC C++ plugin found at: ${gRPC_CPP_PLUGIN_EXECUTABLE}")

# ── Per-proto-file code generation ───────────────────────────────────────
foreach (proto_file IN LISTS PROTO_FILES)
    get_filename_component(proto_name_we ${proto_file} NAME_WE)
    get_filename_component(proto_rel_path ${proto_file} PATH)
    file(RELATIVE_PATH proto_sub_dir ${PROTO_SOURCE_DIR} ${proto_rel_path})

    set(pb_cc   "${PROTO_OUTPUT_DIR}/${proto_sub_dir}/${proto_name_we}.pb.cc")
    set(pb_h    "${PROTO_OUTPUT_DIR}/${proto_sub_dir}/${proto_name_we}.pb.h")
    set(grpc_pb_cc "${PROTO_OUTPUT_DIR}/${proto_sub_dir}/${proto_name_we}.grpc.pb.cc")
    set(grpc_pb_h  "${PROTO_OUTPUT_DIR}/${proto_sub_dir}/${proto_name_we}.grpc.pb.h")

    file(MAKE_DIRECTORY "${PROTO_OUTPUT_DIR}/${proto_sub_dir}")

    add_custom_command(
        OUTPUT ${pb_cc} ${pb_h} ${grpc_pb_cc} ${grpc_pb_h}
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS
            --cpp_out=${PROTO_OUTPUT_DIR}
            --grpc_out=${PROTO_OUTPUT_DIR}
            --plugin=protoc-gen-grpc=${gRPC_CPP_PLUGIN_EXECUTABLE}
            -I${PROTO_SOURCE_DIR}
            ${proto_file}
        DEPENDS ${proto_file}
        COMMENT "Generating gRPC code for ${proto_file}"
    )

    list(APPEND PROTO_SRCS ${pb_cc} ${grpc_pb_cc})
    list(APPEND PROTO_HDRS ${pb_h} ${grpc_pb_h})
endforeach ()

add_custom_target(
    ${PROTOBUF_PROJECT}
    ALL
    DEPENDS ${PROTO_SRCS} ${PROTO_HDRS}
    COMMENT "Generating gRPC code from proto files"
)

# ── Library ──────────────────────────────────────────────────────────────
set(GRPC_SERVICE grpc_service)

add_library(${GRPC_SERVICE} ${PROTO_SRCS} ${PROTO_HDRS})
add_dependencies(${GRPC_SERVICE} ${PROTOBUF_PROJECT})

target_include_directories(${GRPC_SERVICE} PUBLIC
    ${PROJECT_SOURCE_DIR}/proto
)

target_link_libraries(${GRPC_SERVICE} PRIVATE
    gRPC::gpr
    gRPC::grpc
    gRPC::grpc++
    gRPC::grpc++_alts
)