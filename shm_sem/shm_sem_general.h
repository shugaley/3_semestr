#ifndef SHM_SEM_SHM_SEM_GENERAL_H
#define SHM_SEM_SHM_SEM_GENERAL_H

#include <sys/sem.h>


// Shell funcs {

char* CreateSharedMemory (const char* path, const int prog_id, int* shmid);

void  CreateSemaphores(const char* path, const int prog_id, int* semid);

// } Shell funcs

#endif //SHM_SEM_SHM_SEM_GENERAL_H