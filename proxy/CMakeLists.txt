cmake_minimum_required(VERSION 3.15)
project(proxy C)

set(CMAKE_C_STANDARD 11)

# set(CMAKE_C_COMPILER "/usr/bin/clang")

include_directories(numbers)

add_executable(proxy main.c
                     proxy.c proxy.h)
add_subdirectory(numbers)

target_link_libraries(proxy numbers "-lm")
