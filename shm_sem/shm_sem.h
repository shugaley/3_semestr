#ifndef SHM_SEM_SHM_SEM_H
#define SHM_SEM_SHM_SEM_H

#include <sys/types.h>

#include "shm_sem_general.h"

static const char FTOK_PATHNAME[] = __FILE__;
static const int  FTOK_PROJ_ID    = 0;

static const size_t SIZE_SHARED_MEMORY = 4096;
static const size_t N_SEMAPHORES = 7;

enum NumSemaphores {
    SEM_READER_EXIST   = 0,
    SEM_WRITER_EXIST   = 1,
    SEM_READER_READY   = 2,
    SEM_WRITER_READY   = 3,
    SEM_READ_TO_SHM    = 4,
    SEM_WRITE_FROM_SHM = 5,
    SEM_N_ACTIVE_PROC  = 6,
};


void WriteFromSharedMemory();
void ReadToSharedMemory(const char* path_input);

#endif //SHM_SEM_SHM_SEM_H
