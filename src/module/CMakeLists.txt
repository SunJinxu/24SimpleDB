cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(db db.c vm.c compiler.c pager.c)
target_include_directories(db PUBLIC ${CMAKE_SOURCE_DIR}/src/module)