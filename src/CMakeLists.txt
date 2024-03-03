cmake_minimum_required(VERSION 3.11)

set(CMAKE_BUILD_TYPE Debug)

add_executable(main main.c module/compiler.c module/vm.c module/db.c)
target_link_libraries(main PRIVATE db)

target_include_directories(main PUBLIC module)

add_subdirectory(module)