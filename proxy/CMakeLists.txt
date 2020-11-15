cmake_minimum_required(VERSION 3.17)
set(CMAKE_C_COMPILER clang)
project(proxy C)

set(CMAKE_C_STANDARD 11)

include_directories(numbers)

add_executable(proxy main.c
                     proxy.c proxy.h)
add_subdirectory(numbers)

target_link_libraries(proxy numbers)