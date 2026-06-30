# ==============================================================================
#  Starter: Tracing -- starter_tracing
# ==============================================================================

set(STARTER_TRACING starter_tracing)

# -- Sources -------------------------------------------------------------------
file(GLOB_RECURSE TRACING_SRC_FILES src/*.cc)

# -- Library -------------------------------------------------------------------
add_library(${STARTER_TRACING} STATIC
    ${TRACING_SRC_FILES}
)

target_include_directories(${STARTER_TRACING} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${STARTER_TRACING} PUBLIC
    common_pkg
    glog_service
)

# -- Install -------------------------------------------------------------------
install(TARGETS ${STARTER_TRACING}
    EXPORT cppforgeTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/cppforge
    DESTINATION include
)
