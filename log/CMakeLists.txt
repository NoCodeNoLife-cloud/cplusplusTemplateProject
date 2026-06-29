# ══════════════════════════════════════════════════════════════════════════
#  Logging library — glog_service
# ══════════════════════════════════════════════════════════════════════════
#
# Logging service built on Google glog.  Wraps glog with YAML-based
# configuration and provides a project-specific logging facade.
#
# ══════════════════════════════════════════════════════════════════════════

# ── Target ───────────────────────────────────────────────────────────────
set(LOG_SERVICE glog_service)

# ── Sources ──────────────────────────────────────────────────────────────
file(GLOB_RECURSE GLOG_SRC_FILES
    src/*.cc
)

# ── Library ──────────────────────────────────────────────────────────────
add_library(${LOG_SERVICE} ${GLOG_SRC_FILES})

target_include_directories(${LOG_SERVICE} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${LOG_SERVICE} PUBLIC
    common_pkg
)

target_link_libraries(${LOG_SERVICE} PRIVATE
    glog::glog
    yaml-cpp::yaml-cpp
    fmt::fmt
)

# ── Install ───────────────────────────────────────────────────────────────
install(TARGETS ${LOG_SERVICE}
    EXPORT cppforgeTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/cppforge
    DESTINATION include
)
