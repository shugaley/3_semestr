#ifndef SHM_SEM_SHM_SEM_GENERAL_H
#define SHM_SEM_SHM_SEM_GENERAL_H

#include <sys/types.h>

// Shell funcs {

char* ConstructSharedMemory(const char* path, int prog_id, size_t size, int* shmid);
void   DestructSharedMemory(const char* shmaddr, int shmid);

void  CreateSemaphores     (const char* path, int prog_id, size_t nsops, int* semid);
void  Semop(int semid, short num_semaphore, short n)

// } Shell funcs

#endif //SHM_SEM_SHM_SEM_GENERAL_H