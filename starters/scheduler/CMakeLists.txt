# ==============================================================================
#  Starter: Scheduler -- starter_scheduler
# ==============================================================================

set(STARTER_SCHEDULER starter_scheduler)

# -- Sources -------------------------------------------------------------------
file(GLOB_RECURSE SCHEDULER_SRC_FILES src/*.cc)

# -- Library -------------------------------------------------------------------
add_library(${STARTER_SCHEDULER} STATIC
    ${SCHEDULER_SRC_FILES}
)

target_include_directories(${STARTER_SCHEDULER} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${STARTER_SCHEDULER} PUBLIC
    common_pkg
    glog_service
)

# -- Install -------------------------------------------------------------------
install(TARGETS ${STARTER_SCHEDULER}
    EXPORT cppforgeTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/cppforge
    DESTINATION include
)
