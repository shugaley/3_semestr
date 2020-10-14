
#include "shm_sem_general.h"

#include "assert.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/types.h"

static const size_t SIZE_SHARED_MEMORY = 4096;
static const int SHMGET_SHMFLG         = 0666;

// Shell funcs {

char* GetSharedMemory(const char* path, const int prog_id)
{
    assert(path);

    key_t key = ftok(path, prog_id);
    if(key < 0) {
        perror("Error ftok()");
        exit(EXIT_FAILURE);
    }

    int shmid = shmget(key, SIZE_SHARED_MEMORY, SHMGET_SHMFLG | IPC_CREAT);
    if(shmid < 0) {
        perror("Error shmget()");
        exit(EXIT_FAILURE);
    }

    char* shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory < 0) {
        perror("Error shmat()");
        exit(EXIT_FAILURE);
    }

    return shared_memory;
}


// } Shell funcs
