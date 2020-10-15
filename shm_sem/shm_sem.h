#ifndef SHM_SEM_SHM_SEM_H
#define SHM_SEM_SHM_SEM_H

#include <sys/types.h>

static const char FTOK_PATHNAME[] = __FILE__;
static const int  FTOK_PROJ_ID    = 0;

static const size_t SIZE_SHARED_MEMORY = 4096;
static const size_t N_SEMAPHORES       = 4;

enum NumSemaphores {
    NUM_SEMAPHORES_EXIST_WRITER = 0,
    NUM_SEMAPHORES_EXIST_READER = 1,
};

void WriteSharedMemory(const char* pathInput);
void ReadSharedMemory ();

#endif //SHM_SEM_SHM_SEM_H
