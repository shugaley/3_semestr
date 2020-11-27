cmake_minimum_required(VERSION 3.15)
set(CMAKE_C_COMPILER clang)
project(signals C)

set(CMAKE_C_STANDARD 11)

add_executable(signals main.c
                       signals.h signals.c)
