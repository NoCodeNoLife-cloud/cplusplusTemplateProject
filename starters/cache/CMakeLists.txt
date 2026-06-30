# ==============================================================================
#  Starter: Cache -- starter_cache
# ==============================================================================

set(STARTER_CACHE starter_cache)

# -- Sources -------------------------------------------------------------------
file(GLOB_RECURSE CACHE_SRC_FILES src/*.cc)

# -- Library -------------------------------------------------------------------
add_library(${STARTER_CACHE} STATIC
    ${CACHE_SRC_FILES}
)

target_include_directories(${STARTER_CACHE} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${STARTER_CACHE} PUBLIC
    common_pkg
    glog_service
)

# -- Install -------------------------------------------------------------------
install(TARGETS ${STARTER_CACHE}
    EXPORT cppforgeTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/cppforge
    DESTINATION include
)
