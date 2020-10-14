
#include "shm_sem.h"

#include "stdio.h"
#include "stdlib.h"
#include "sys/ipc.h"
#include "sys/shm.h"

#include "shm_sem_general.h"

//Debug
#include "unistd.h"

void ReadSharedMemory ()
{
    int shmid = 0;
    char* shared_memory = GetSharedMemory(FTOK_PATHNAME, FTOK_PROJ_ID, &shmid);


    int ret_shmctl = shmctl(shmid, IPC_RMID, NULL);
    if (ret_shmctl < 0) {
        perror("Error shmctl()");
        exit(EXIT_FAILURE);
    }
}
