cmake_minimum_required(VERSION 3.17)
project(shm_sem C)

set(CMAKE_C_STANDARD 11)

add_executable(shm_sem_reader reader_main.c
                              shm_sem.h reader.c)
add_executable(shm_sem_writer writer_main.c
                              shm_sem.h writer.c)