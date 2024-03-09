cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

add_executable(dbTest db_test.cpp)

target_link_libraries(dbTest components gtest_main gmock_main)
target_include_directories(dbTest PUBLIC ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src)