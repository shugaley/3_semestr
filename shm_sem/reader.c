
#include "shm_sem.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "shm_sem_general.h"

//Debug
#include "unistd.h"

void ReadData(const char* shmaddr, int semid);


void ReadSharedMemory ()
{
    int semid = 0;
    CreateSemaphores(FTOK_PATHNAME, FTOK_PROJ_ID, N_SEMAPHORES, &semid);

    int shmid = 0;
    char* shmaddr = ConstructSharedMemory(FTOK_PATHNAME, FTOK_PROJ_ID,
                                                SIZE_SHARED_MEMORY, &shmid);

    ReadData(shmaddr, semid);
    DestructSharedMemory(shmaddr, shmid);

    errno = 0;
    int ret_semctl = semctl(semid, 0, IPC_RMID, NULL);
    if (ret_semctl < 0) {
        perror("Error ret_semctl()");
        exit(EXIT_FAILURE);
    }
}


void ReadData(const char* shmaddr, int semid)
{
    assert(shmaddr);

    while(1) {
        Semop(semid, NUM_SEMAPHORES_IS_FULL, -1, 0);
        Semop(semid, NUM_SEMAPHORES_MUTEX,   -1, 0);

        if(*shmaddr == '\0')
            break;
        printf("%s", shmaddr);
        fflush(stdout);

        Semop(semid, NUM_SEMAPHORES_MUTEX,    1, 0);
        Semop(semid, NUM_SEMAPHORES_IS_EMPTY, 1, 0);
    }
}
