
#include "shm_sem.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

void WriteData(const char* shared_memory, int id_sem);

void WriteFromSharedMemory()
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

    DumpSemaphores(id_sem, N_SEMAPHORES, "writer begin");

    // Block other writers
    struct sembuf sops_MutexWriters[2] = {
            {SEM_WRITER_EXIST, 0, 0},
            {SEM_WRITER_EXIST, 1, SEM_UNDO},
    };
    ret = semop(id_sem, sops_MutexWriters, 2);
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

    AssignSem(id_sem, SEM_READ_TO_SHM, 2);

    DumpSemaphores(id_sem, N_SEMAPHORES, "Writer after wainting other");
    // if (writer == 2) current reader is alive
    // not block reader if writer die
    struct sembuf sops_DefenceDeadlock[2] = {
            {SEM_WRITER_READY,  1, SEM_UNDO},
            {SEM_READ_TO_SHM,  -1, SEM_UNDO},
    };
    ret = semop(id_sem, sops_DefenceDeadlock, 2);
    if (ret < 0) {
        perror("Error semop");
        exit(EXIT_FAILURE);
    }

    DumpSemaphores(id_sem, N_SEMAPHORES, "Writer waiting pair");
    // Check that reader exists
    // Mark  that writer is alive
    struct sembuf sops_WaitingPair[3] = {
            {SEM_READER_READY, -1, 0},
            {SEM_READER_READY,  1, 0},
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
    WriteData(shared_memory, id_sem);

    DestructSharedMemory(shared_memory, id_shm);

    DumpSemaphores(id_sem, N_SEMAPHORES, "writer before clear");
    // Clear semaphores
    struct sembuf sops_FinishWriting[3] = {
            {SEM_WRITER_READY,  -1, SEM_UNDO},
            {SEM_N_ACTIVE_PROC, -1, SEM_UNDO},
            {SEM_WRITER_EXIST,  -1, SEM_UNDO},
    };
    ret = semop(id_sem, sops_FinishWriting, 3);
    if (ret < 0) {
        perror("Error semop");
        exit(EXIT_FAILURE);
    }
}


void WriteData(const char* shared_memory, int id_sem)
{
    assert(shared_memory);

    size_t n_bytes = -1;
    while(n_bytes != 0) {
        int ret = 0;

        struct sembuf sops_BeforeWrite[1] =
                {SEM_WRITE_FROM_SHM, -1, 0};
        ret = semop(id_sem, sops_BeforeWrite, 1);
        if (ret < 0) {
            perror("Error semop");
            exit(EXIT_FAILURE);
        }

        n_bytes = *(ssize_t*)shared_memory;
        ret = write(STDOUT_FILENO, shared_memory + sizeof(n_bytes), n_bytes);
        if (ret < 0) {
            perror("Error write");
            exit(EXIT_FAILURE);
        }


        struct sembuf sops_AfterWrite[1] =
                {SEM_READ_TO_SHM, 1, 0};
        ret = semop(id_sem, sops_AfterWrite, 1);
        if (ret < 0) {
            perror("Error semop");
            exit(EXIT_FAILURE);
        }
    }
}
