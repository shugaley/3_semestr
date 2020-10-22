
#include "shm_sem.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void WriteData(const char* path_input, char* shmaddr, int semid);


void WriteSharedMemory(const char* path_input)
{
    assert(path_input);

    errno = 0;
    key_t key = ftok(FTOK_PATHNAME, FTOK_PROJ_ID);
    if (key < 0) {
        perror("Error ftok()");
        exit(EXIT_FAILURE);
    }

    int semid = 0;
    CreateSemaphores(key, N_SEMAPHORES, SEM_INIT_DATA, &semid);
    DumpSemaphores(semid, N_SEMAPHORES, "writer begin");
    Semop(semid, NUM_SEMAPHORES_IS_FREE_WRITER, -1, SEM_UNDO);

    int shmid = 0;
    char* shmaddr = ConstructSharedMemory(key, SIZE_SHARED_MEMORY, &shmid);

    WriteData(path_input, shmaddr, semid);

    //delete sem???
    Semop(semid, NUM_SEMAPHORES_IS_FREE_WRITER, 1, SEM_UNDO);
}


void WriteData(const char* path_input, char* shmaddr, int semid)
{
    assert(path_input);
    assert(shmaddr);

    errno = 0;
    (void) umask(0);
    int fd = open(path_input, O_RDONLY);
    if (fd < 0) {
        perror("Error open");
        exit(EXIT_FAILURE);
    }

    ssize_t ret_read = 1;
    while (ret_read != 0) {

        DumpSemaphores(semid, N_SEMAPHORES, "writer 1");

        Semop(semid, NUM_SEMAPHORES_IS_EMPTY, -1, 0);
        Semop(semid, NUM_SEMAPHORES_MUTEX,    -1, SEM_UNDO);

        DumpSemaphores(semid, N_SEMAPHORES, "writer 2");

        errno = 0;
        ret_read = read(fd, shmaddr, SIZE_SHARED_MEMORY);
        if (ret_read < 0) {
            perror("Error read()");
            exit(EXIT_FAILURE);
        }

        if (ret_read < SIZE_SHARED_MEMORY)
            shmaddr[ret_read] = '\0';

        DumpSemaphores(semid, N_SEMAPHORES, "writer 3");

        Semop(semid, NUM_SEMAPHORES_MUTEX,   1, SEM_UNDO);
        Semop(semid, NUM_SEMAPHORES_IS_FULL, 1, 0);

        DumpSemaphores(semid, N_SEMAPHORES, "writer 4");
    }
    close(fd);
}
