#ifndef SHM_SEM_SHM_SEM_GENERAL_H
#define SHM_SEM_SHM_SEM_GENERAL_H

#include <sys/sem.h>

struct Semaphores {
    const size_t n;
    struct sembuf* existingWriter;
    struct sembuf* existingReader;
    struct sembuf* writer;
    struct sembuf* reader;
};


// Shell funcs {

char*             CreateSharedMemory (const char* path, const int prog_id, int* shmid);

struct Semaphores ConstructSemaphores(const char* path, const int prog_id, int* semid);
//void DestructSemaphores(struct Semaphores* semaphores, int* semid);

// } Shell funcs

#endif //SHM_SEM_SHM_SEM_GENERAL_H