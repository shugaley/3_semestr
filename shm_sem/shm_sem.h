#ifndef SHM_SEM_SHM_SEM_H
#define SHM_SEM_SHM_SEM_H

#include <sys/types.h>

#include "shm_sem_general.h"

static const char FTOK_PATHNAME[] = __FILE__;
static const int  FTOK_PROJ_ID    = 0;

static const size_t SIZE_SHARED_MEMORY = 4096;
#define N_SEMAPHORES 5

enum NumSemaphores {
    NUM_SEMAPHORES_IS_FREE_WRITER = 0,
    NUM_SEMAPHORES_IS_FREE_READER = 1,
    NUM_SEMAPHORES_MUTEX          = 2,
    NUM_SEMAPHORES_IS_EMPTY       = 3,
    NUM_SEMAPHORES_IS_FULL        = 4,
};

static const struct SemaphoreData SEM_INIT_DATA[N_SEMAPHORES] =
        {NUM_SEMAPHORES_IS_FREE_WRITER, 1,
         NUM_SEMAPHORES_IS_FREE_READER, 1,
         NUM_SEMAPHORES_MUTEX,          1,
         NUM_SEMAPHORES_IS_EMPTY,       1,
         NUM_SEMAPHORES_IS_FULL,        0};


void WriteSharedMemory(const char* path_input);
void ReadSharedMemory ();

#endif //SHM_SEM_SHM_SEM_H
