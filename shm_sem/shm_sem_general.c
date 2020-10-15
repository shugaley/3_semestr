
#include "shm_sem_general.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

static const int SHMGET_SHMFLG = 0666;
static const int SEMGET_SEMFLG = 0666;

// Shell funcs {

char* ConstructSharedMemory(const char* path, int prog_id, size_t size, int* shmid)
{
    assert(path);

    errno = 0;
    key_t key = ftok(path, prog_id);
    if (key < 0) {
        perror("Error ftok()");
        exit(EXIT_FAILURE);
    }

    errno = 0;
    int ret_shmget = shmget(key, size, SHMGET_SHMFLG | IPC_CREAT);
    if (ret_shmget < 0) {
        perror("Error shmget()");
        exit(EXIT_FAILURE);
    }

    if(shmid)
        *shmid = ret_shmget;

    errno = 0;
    char* shared_memory = shmat(ret_shmget, NULL, 0);
    if (shared_memory < 0) {
        perror("Error shmat()");
        exit(EXIT_FAILURE);
    }

    return shared_memory;
}


void DestructSharedMemory(const char* shmaddr, int shmid)
{
    assert(shmaddr);

    errno = 0;
    int ret_shmdt = shmdt(shmaddr);
    if (shmaddr < 0) {
        perror("Error smaddr()");
        exit(EXIT_FAILURE);
    }

    errno = 0;
    int ret_shmctl = shmctl(shmid, IPC_RMID, NULL);
    if (ret_shmctl < 0) {
        perror("Error shmctl()");
        exit(EXIT_FAILURE);
    }
}


void CreateSemaphores(const char* path, int prog_id, size_t nsops, int* semid)
{
    assert(path);

    errno = 0;
    key_t key = ftok(path, prog_id);
    if (key < 0) {
        perror("Error ftok()");
        exit(EXIT_FAILURE);
    }

    errno = 0;
    int ret_semget = semget(key, nsops, SEMGET_SEMFLG | IPC_CREAT);
    if (ret_semget < 0) {
        perror("Error semget()");
        exit(EXIT_FAILURE);
    }

    if (semid)
        *semid = ret_semget;
}


void Semop(int semid, short num_semaphore, short n)
{
    struct sembuf semaphore;
    semaphore.sem_num = num_semaphore;
    semaphore.sem_op  = n;
    semaphore.sem_flg = SEM_UNDO;

    errno = 0;
    int ret_semop = semop(semid, &semaphore, 1);
    if (ret_semop < 0) {
        perror("Error semop");
        exit(EXIT_FAILURE);
    }
}

// } Shell funcs
