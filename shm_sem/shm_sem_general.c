
#include "shm_sem_general.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

static const size_t SIZE_SHARED_MEMORY = 4096;
static const int SHMGET_SHMFLG         = 0666;

static const size_t N_SEMAPHORES = 4;
static const int SEMGET_SEMFLG   = 0666;

// Shell funcs {

char* CreateSharedMemory(const char* path, const int prog_id, int* shmid)
{
    assert(path);

    key_t key = ftok(path, prog_id);
    if (key < 0) {
        perror("Error ftok()");
        exit(EXIT_FAILURE);
    }

    int ret_shmget = shmget(key, SIZE_SHARED_MEMORY, SHMGET_SHMFLG | IPC_CREAT);
    if (ret_shmget < 0) {
        perror("Error shmget()");
        exit(EXIT_FAILURE);
    }

    if(shmid)
        *shmid = ret_shmget;

    char* shared_memory = shmat(ret_shmget, NULL, 0);
    if (shared_memory < 0) {
        perror("Error shmat()");
        exit(EXIT_FAILURE);
    }

    return shared_memory;
}


struct Semaphores ConstructSemaphores(const char* path, const int prog_id, int* semid)
{
    assert(path);

    key_t key = ftok(path, prog_id);
    if (key < 0) {
        perror("Error ftok()");
        exit(EXIT_FAILURE);
    }

    int ret_semget = semget(key, N_SEMAPHORES, SEMGET_SEMFLG | IPC_CREAT);
    if (ret_semget < 0) {
        perror("Error semget()");
        exit(EXIT_FAILURE);
    }

    if (semid)
        *semid = ret_semget;

    struct sembuf* sops = (struct sembuf*)calloc(N_SEMAPHORES, sizeof(*sops));
    struct Semaphores semaphores = {N_SEMAPHORES, &sops[0], &sops[1],
                                                  &sops[2], &sops[3]  };
    return semaphores;
}

//void DestructSemaphores(struct Semaphores* semaphores, int* semid);

// } Shell funcs
