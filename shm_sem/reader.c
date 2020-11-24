
#include "shm_sem.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>


void ReadData(const char* shmaddr, int semid);


void ReadSharedMemory ()
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

    // Block other reader
    struct sembuf sops_MutexReaders[2] = {
            {SEM_READER, 0, 0},
            {SEM_READER, 1, SEM_UNDO}
    };
    ret = semop(id_sem, sops_MutexReaders, 2);
    if (ret < 0) {
        perror("Error semop");
        exit(EXIT_FAILURE);
    }

    // Wait end of previous transfer
    struct sembuf sops_WaitingPrevious[1] =
            {SEM_N_ACTIVE, 0, 0};
    ret = semop(id_sem, sops_WaitingPrevious, 1);
    if (ret < 0) {
        perror("Error semop");
        exit(EXIT_FAILURE);
    }

    int id_shm = 0;
    char* shared_memory = ConstructSharedMemory(key, SIZE_SHARED_MEMORY,
                                                &id_sem);







}


void ReadData(const char* shmaddr, int semid)
{

}
