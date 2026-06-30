# ==============================================================================
#  Starter: Security -- starter_security
# ==============================================================================

set(STARTER_SECURITY starter_security)

# -- Sources -------------------------------------------------------------------
file(GLOB_RECURSE SECURITY_SRC_FILES src/*.cc)

# -- Library -------------------------------------------------------------------
add_library(${STARTER_SECURITY} STATIC
    ${SECURITY_SRC_FILES}
)

target_include_directories(${STARTER_SECURITY} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${STARTER_SECURITY} PUBLIC
    common_pkg
    glog_service
)

target_link_libraries(${STARTER_SECURITY} PRIVATE
    OpenSSL::SSL OpenSSL::Crypto
    rapidjson
)

# -- Install -------------------------------------------------------------------
install(TARGETS ${STARTER_SECURITY}
    EXPORT cppforgeTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/cppforge
    DESTINATION include
)