cmake_minimum_required(VERSION 3.30)
project(cppforge_sample LANGUAGES CXX)

# Find cppforge from build tree (CMAKE_PREFIX_PATH) or install tree
find_package(cppforge CONFIG REQUIRED)

add_executable(sample_app main.cpp)
target_link_libraries(sample_app PRIVATE
    cppforge::common_pkg
    cppforge::glog_service
)
