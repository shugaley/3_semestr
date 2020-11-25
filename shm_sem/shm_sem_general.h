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

void AssignSem(int id_sem, int num_sem, short value);

void DumpSemaphores(int semid, size_t nsops, const char* str);

// } Shell funcs

#endif //SHM_SEM_SHM_SEM_GENERAL_H