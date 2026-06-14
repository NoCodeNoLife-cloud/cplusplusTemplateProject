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
    src/*.hpp
)

# ── Library ──────────────────────────────────────────────────────────────
add_library(${LOG_SERVICE} ${GLOG_SRC_FILES})

target_include_directories(${LOG_SERVICE} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/src
)

target_link_libraries(${LOG_SERVICE} PRIVATE
    glog::glog
    yaml-cpp::yaml-cpp
    fmt::fmt
)