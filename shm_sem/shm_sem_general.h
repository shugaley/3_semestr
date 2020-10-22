#ifndef SHM_SEM_SHM_SEM_GENERAL_H
#define SHM_SEM_SHM_SEM_GENERAL_H

#include <sys/types.h>

// Shell funcs {

struct SemaphoreData {
    size_t num;
    short  value;
};

char* ConstructSharedMemory(key_t key, size_t size, int* shmid);
void   DestructSharedMemory(const char* shmaddr, int shmid);

void  CreateSemaphores(key_t key,
                       size_t nsops,
                       const struct SemaphoreData* sem_initData,
                       int* semid);

void  InitSemaphores(int semid, size_t nsops,
                     const struct SemaphoreData* sem_initData);

void  Semop(int semid, short sem_num, short sem_op, short sem_flg);

void DumpSemaphores(int semid, size_t nsops, const char* str);

// } Shell funcs

#endif //SHM_SEM_SHM_SEM_GENERAL_H