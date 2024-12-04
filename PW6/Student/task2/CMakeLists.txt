project(task2)

cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 14)

# required by clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# enable debug mode
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -g -pedantic")

# set(SYSYF_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)

# # include generated files in project environment
# include_directories(${SYSYF_SOURCE_DIR}include)
# include_directories(${SYSYF_SOURCE_DIR}include/SysYFIR)

add_subdirectory(demo)
add_subdirectory(cpp)
