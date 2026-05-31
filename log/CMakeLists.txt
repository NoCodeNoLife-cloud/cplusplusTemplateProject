# Project
set(LOG_SERVICE glog_service)

file(GLOB_RECURSE GLOG_SRC_FILES
        src/*.cc
        src/*.hpp
)

# Creating a Common Library Target
add_library(${LOG_SERVICE} ${GLOG_SRC_FILES})

# Set the header file path
target_include_directories(${LOG_SERVICE} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/src
)

# Link required libraries
target_link_libraries(${LOG_SERVICE} PRIVATE
        glog::glog
        yaml-cpp::yaml-cpp
        fmt::fmt
)