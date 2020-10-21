
#include "shm_sem.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>

//Debug
#include "unistd.h"

void ReadData(const char* shmaddr, int semid);


void ReadSharedMemory ()
{
    int semid = 0;
    CreateSemaphores(FTOK_PATHNAME, FTOK_PROJ_ID, N_SEMAPHORES,
                     SEM_INIT_DATA,  &semid);

    DumpSemaphores(semid, N_SEMAPHORES);

    int shmid = 0;
    char* shmaddr = ConstructSharedMemory(FTOK_PATHNAME, FTOK_PROJ_ID,
                                          SIZE_SHARED_MEMORY, &shmid);

    ReadData(shmaddr, semid);
    DestructSharedMemory(shmaddr, shmid);
}


void ReadData(const char* shmaddr, int semid)
{
    assert(shmaddr);

    while(1) {
        DumpSemaphores(semid, N_SEMAPHORES);

        Semop(semid, NUM_SEMAPHORES_IS_FULL, -1, 0);
        Semop(semid, NUM_SEMAPHORES_MUTEX,   -1, 0);

        DumpSemaphores(semid, N_SEMAPHORES);

        if(*shmaddr == '\0') {
            Semop(semid, NUM_SEMAPHORES_MUTEX,    1, 0);
            Semop(semid, NUM_SEMAPHORES_IS_EMPTY, 1, 0);
            break;
        }

        printf("%s", shmaddr);
        fflush(stdout);

        DumpSemaphores(semid, N_SEMAPHORES);

        Semop(semid, NUM_SEMAPHORES_MUTEX,    1, 0);
        Semop(semid, NUM_SEMAPHORES_IS_EMPTY, 1, 0);

        DumpSemaphores(semid, N_SEMAPHORES);
    }
}
