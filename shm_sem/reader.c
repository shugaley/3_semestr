
#include "shm_sem.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

void ReadData(const char* path_input, char* shared_memory, int id_sem);


void ReadToSharedMemory(const char* path_input)
{
    errno = 0;
    key_t key = ftok(FTOK_PATHNAME, FTOK_PROJ_ID);
    if (key < 0) {
        perror("Error ftok()");
        exit(EXIT_FAILURE);
    }

    int id_sem = semget(key, N_SEMAPHORES, 0666 | IPC_CREAT);
    if (id_sem < 0) {
        perror("Error semget");
        exit(EXIT_FAILURE);
    }

    int ret = 0;

    // Block other readers
    struct sembuf sops_MutexReaders[2] = {
            {SEM_READER_EXIST, 0, 0},
            {SEM_READER_EXIST, 1, SEM_UNDO}
    };
    ret = semop(id_sem, sops_MutexReaders, 2);
    if (ret < 0) {
        perror("Error semop");
        exit(EXIT_FAILURE);
    }

    // Wait end of previous transfer
    struct sembuf sops_WaitingPrevious[1] =
            {SEM_N_ACTIVE_PROC, 0, 0};
    ret = semop(id_sem, sops_WaitingPrevious, 1);
    if (ret < 0) {
        perror("Error semop");
        exit(EXIT_FAILURE);
    }

    AssignSem(id_sem, SEM_WRITE_FROM_SHM, 1);

    // Reader already init SEM_WRITE_FROM_SHM
    // Not block writer if reader die
    struct sembuf sops_DefenceDeadlock[2] = {
            {SEM_READER_READY,    1, SEM_UNDO},
            {SEM_WRITE_FROM_SHM, -1, SEM_UNDO},
    };
    ret = semop(id_sem, sops_DefenceDeadlock, 2);
    if (ret < 0) {
        perror("Error semop");
        exit(EXIT_FAILURE);
    }

    // Check that writer exists
    // Mark  that reader is alive
    struct sembuf sops_WaitingPair[3] = {
            {SEM_WRITER_READY, -1, 0},
            {SEM_WRITER_READY,  1, 0},
            {SEM_N_ACTIVE_PROC, 1, SEM_UNDO},
    };
    ret = semop(id_sem, sops_WaitingPair, 3);
    if (ret < 0) {
        perror("Error semop");
        exit(EXIT_FAILURE);
    }

    int id_shm = 0;
    char* shared_memory = ConstructSharedMemory(key, SIZE_SHARED_MEMORY,
                                                &id_shm);
    ReadData(path_input, shared_memory, id_sem);

    // Clear semaphores
    struct sembuf sops_FinishReading[3] = {
            {SEM_READER_READY,  -1, SEM_UNDO},
            {SEM_N_ACTIVE_PROC, -1, SEM_UNDO},
            {SEM_READER_EXIST,  -1, SEM_UNDO},
    };
    ret = semop(id_sem, sops_FinishReading, 3);
    if (ret < 0) {
        perror("Error semop");
        exit(EXIT_FAILURE);
    }
}


void ReadData(const char* path_input, char* shared_memory, int id_sem)
{
    assert(path_input);
    assert(shared_memory);

    int fd_input = open(path_input, O_RDONLY);
    if (fd_input < 0) {
        perror("Error open");
        exit(EXIT_FAILURE);
    }

    ssize_t n_bytes = -1;
    while(n_bytes != 0) {
        int ret = 0;

        struct sembuf sops_BeforeRead[1] =
                {SEM_READ_TO_SHM, -1, 0};
        ret = semop(id_sem, sops_BeforeRead, 1);
        if (ret < 0) {
            perror("Error semop");
            exit(EXIT_FAILURE);
        }

        n_bytes = read(fd_input, shared_memory + sizeof(n_bytes),
                       SIZE_SHARED_MEMORY - sizeof(n_bytes));
        if (n_bytes < 0) {
            perror("Error read");
            exit(EXIT_FAILURE);
        }
        *(ssize_t*)shared_memory = n_bytes;

        struct sembuf sops_AfterRead[1] =
                {SEM_WRITE_FROM_SHM, 1, 0};
        ret = semop(id_sem, sops_AfterRead, 1);
        if (ret < 0) {
            perror("Error semop");
            exit(EXIT_FAILURE);
        }
    }
}
