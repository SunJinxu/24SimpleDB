cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(components db.c vm.c compiler.c pager.c cursor.c b_tree.c util.c)
target_include_directories(components PUBLIC ${CMAKE_SOURCE_DIR}/src/module)