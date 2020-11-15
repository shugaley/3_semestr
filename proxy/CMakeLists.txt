cmake_minimum_required(VERSION 3.17)
set(CMAKE_C_COMPILER clang)
project(proxy C)

set(CMAKE_C_STANDARD 11)

add_executable(proxy main.c
                     proxy.c proxy.h)