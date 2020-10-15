#ifndef SHM_SEM_SHM_SEM_H
#define SHM_SEM_SHM_SEM_H

#include <sys/types.h>

static const char FTOK_PATHNAME[] = __FILE__;
static const int  FTOK_PROJ_ID    = 0;

static const size_t SIZE_SHARED_MEMORY = 4096;
static const size_t N_SEMAPHORES       = 3;

enum NumSemaphores {
    NUM_SEMAPHORES_EXIST_WRITER = 0,
    NUM_SEMAPHORES_EXIST_READER = 1,
    NUM_SEMAPHORES_MUTEX        = 2,
};

void WriteSharedMemory(const char* path_input);
void ReadSharedMemory ();

#endif //SHM_SEM_SHM_SEM_H
