cmake_minimum_required(VERSION 3.11)

set(CMAKE_BUILD_TYPE Debug)

add_executable(db main.c)
target_link_libraries(db PRIVATE components)

target_include_directories(db PUBLIC module)

add_subdirectory(module)