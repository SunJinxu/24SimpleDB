cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

add_executable(dbTest db_test.cpp)
target_link_libraries(dbTest gtest_main)
target_include_directories(dbTest PRIVATE ${GTEST_INCLUDE_DIRS})