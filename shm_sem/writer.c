
#include "shm_sem.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "shm_sem_general.h"

//Debug
#include "unistd.h"

void WriteSharedMemory(const char* pathInput)
{
    assert(pathInput);

    int shmid = 0;
    char* shared_memory = CreateSharedMemory(FTOK_PATHNAME, FTOK_PROJ_ID,
                                             SIZE_SHARED_MEMORY, &shmid);
    int semid = 0;
    CreateSemaphores(FTOK_PATHNAME, FTOK_PROJ_ID, N_SEMAPHORES, &semid);



    int ret_shmctl = shmctl(shmid, IPC_RMID, NULL);
    if (ret_shmctl < 0) {
        perror("Error shmctl()");
        exit(EXIT_FAILURE);
    }
}