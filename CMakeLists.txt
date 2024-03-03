cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(24SimpleDb VERSION 1.0)

set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(include/googletest)

include_directories(${CMAKE_SOURCE_DIR}/src/module)