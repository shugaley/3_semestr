#ifndef SHM_SEM_SHM_SEM_H
#define SHM_SEM_SHM_SEM_H

#include <sys/types.h>

#include "shm_sem_general.h"

static const char FTOK_PATHNAME[] = __FILE__;
static const int  FTOK_PROJ_ID    = 0;

static const size_t SIZE_SHARED_MEMORY = 4096;
static const size_t N_SEMAPHORES = 5;

enum NumSemaphores {
    SEM_READER    = 0,
    SEM_WRITER    = 1,
    SEM_READ_SHM  = 2,
    SEM_WRITE_SHM = 3,
    SEM_N_ACTIVE  = 4,
};


void WriteSharedMemory(const char* path_input);
void ReadSharedMemory ();

#endif //SHM_SEM_SHM_SEM_H
