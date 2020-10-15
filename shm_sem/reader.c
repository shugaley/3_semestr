
#include "shm_sem.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "shm_sem_general.h"

//Debug
#include "unistd.h"

void ReadSharedMemory ()
{
    int semid = 0;
    CreateSemaphores(FTOK_PATHNAME, FTOK_PROJ_ID, N_SEMAPHORES, &semid);

    int shmid = 0;
    char* shmaddr = ConstructSharedMemory(FTOK_PATHNAME, FTOK_PROJ_ID,
                                                SIZE_SHARED_MEMORY, &shmid);

    DestructSharedMemory(shmaddr, shmid);

    errno = 0;
    int ret_semctl = semctl(semid, 0, IPC_RMID, NULL);
    if (ret_semctl < 0) {
        perror("Error ret_semctl()");
        exit(EXIT_FAILURE);
    }
}