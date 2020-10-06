cmake_minimum_required(VERSION 3.17)
set(CMAKE_C_COMPILER clang)
project(message C)

set(CMAKE_C_STANDARD 11)

include_directories(numbers)

add_executable(message message_main.c
                       message.h message.c)
add_subdirectory(numbers)

target_link_libraries(message numbers)