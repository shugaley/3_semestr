#ifndef SHM_SEM_SHM_SEM_GENERAL_H
#define SHM_SEM_SHM_SEM_GENERAL_H

#include <sys/types.h>

// Shell funcs {

struct Semaphore;

char* ConstructSharedMemory(const char* path, int prog_id, size_t size, int* shmid);
void   DestructSharedMemory(const char* shmaddr, int shmid);

void  CreateSemaphores     (const char* path, int prog_id, size_t nsops, int* semid);
void  InitSemaphores(int semid, const struct Semaphore* semaphores, size_t nsops);
void  Semop         (int semid, short sem_num, short sem_op, short sem_flg);

void DumpSemaphores(int semid, size_t nsops);

// } Shell funcs

#endif //SHM_SEM_SHM_SEM_GENERAL_H