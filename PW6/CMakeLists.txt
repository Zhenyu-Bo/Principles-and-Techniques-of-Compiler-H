cmake_minimum_required(VERSION 3.5)

project(SysYFCompiler)


set(CMAKE_CXX_STANDARD 14)

# required by clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# if not set, set CMAKE_BUILD_TYPE to Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -Wextra -Wno-unused -Wshadow -g -pedantic")
elseif(CMAKE_BUILD_TYPE STREQUAL "Asan")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -Wextra -Wno-unused -Wshadow -g -pedantic -fsanitize=address -fno-omit-frame-pointer")
else()
    message(WARNING "Unknown build type: ${CMAKE_BUILD_TYPE}. Using default flags.")
endif()

# for ninja color, see https://stackoverflow.com/questions/73349743/ninja-build-system-gcc-clang-doesnt-output-diagnostic-colors
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")

# include generated files in project environment
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/AST)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/ErrorReporter)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/Frontend)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/SysYFIR)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/SysYFIRBuilder)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/AST)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/ErrorReporter)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/Frontend)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/SysYFIR)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/SysYFIRBuilder)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Student/task2)

add_executable(
  SysYFCompiler
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(
  SysYFCompiler
  SysYFIRBuilder
  IRLib
  Driver
  ASTPrinter
  ErrReporter
)
