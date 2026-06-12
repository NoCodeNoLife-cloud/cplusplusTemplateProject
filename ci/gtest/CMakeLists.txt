# Project
# Common shared test support sources
set(CI_GTEST_SHARED_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../client/src/auth/AuthRpcParam.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../server/src/auth/AuthRpcParam.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../server/src/config/ConfigParam.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../client/src/config/ConfigParam.cc
)

# Common include directories
set(CI_GTEST_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../proto
    ${CMAKE_CURRENT_SOURCE_DIR}/../../log/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../client/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../server/src
)

# Common linked libraries
set(CI_GTEST_LIBS
    GTest::gtest
    GTest::gtest_main
    glog::glog
    yaml-cpp::yaml-cpp
    rapidjson
    OpenSSL::SSL OpenSSL::Crypto
    Microsoft::DirectXMath
    magic_enum::magic_enum
    Boost::serialization
    common_pkg
    glog_service
    grpc_service
    fmt::fmt
    unofficial::sqlite3::sqlite3
    unofficial::mysql-connector-cpp::connector
)

# Collect all test .cc/.cpp files
file(GLOB_RECURSE CI_GTEST_ALL_FILES *.cc *.cpp)

# Group by parent directory to create per-directory test executables
# This allows ctest -j N to run tests in parallel across directories
set(CI_GTEST_DIRS)
foreach(FILE ${CI_GTEST_ALL_FILES})
    get_filename_component(FILE_DIR ${FILE} DIRECTORY)
    list(APPEND CI_GTEST_DIRS ${FILE_DIR})
endforeach()
list(REMOVE_DUPLICATES CI_GTEST_DIRS)

foreach(DIR ${CI_GTEST_DIRS})
    # Target name: relative path with / replaced by _
    file(RELATIVE_PATH REL_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${DIR})
    string(REPLACE "/" "_" TARGET_NAME "ut_${REL_DIR}")
    string(REPLACE "\\" "_" TARGET_NAME ${TARGET_NAME})

    # Collect test files in this directory only (non-recursive to avoid
    # duplicate compilation across sibling directories)
    file(GLOB DIR_FILES ${DIR}/*.cc ${DIR}/*.cpp)

    add_executable(${TARGET_NAME} ${DIR_FILES})

    # Add the shared support sources
    target_sources(${TARGET_NAME} PRIVATE ${CI_GTEST_SHARED_SRC})

    target_include_directories(${TARGET_NAME} PRIVATE ${CI_GTEST_INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME} PRIVATE ${CI_GTEST_LIBS})

    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
endforeach()
