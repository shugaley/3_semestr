cmake_minimum_required(VERSION 3.17)
set(CMAKE_C_COMPILER clang)
project(fifo C)

set(CMAKE_C_STANDARD 11)

add_executable(fifo_writer writer_main.c
                      fifo.h fifo_general.h fifo_general.c fifo_writer.c)
add_executable(fifo_reader reader_main.c
                      fifo.h fifo_general.h fifo_general.c fifo_reader.c)