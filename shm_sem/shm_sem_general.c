
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

struct Semaphore {
    size_t num;
    short  value;
};



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
    if (ret_shmdt < 0) {
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


void InitSemaphores(int semid, const struct Semaphore* semaphores, size_t nsops)
{
    assert(semaphores);

    for(size_t i_sem = 0; i_sem < nsops; i_sem++) {
        errno = 0;
        int ret_semctl = semctl(semid,  semaphores[i_sem].num,
                                SETVAL, semaphores[i_sem].value);
        if (ret_semctl < 0) {
            perror("Error semctl in InitSemaphores");
            exit(EXIT_FAILURE);
        }
    }
}


void Semop(int semid, short num_semaphore, short n, short sem_flg)
{
    struct sembuf semaphore = {};
    semaphore.sem_num = num_semaphore;
    semaphore.sem_op  = n;
    semaphore.sem_flg = sem_flg;

    errno = 0;
    int ret_semop = semop(semid, &semaphore, 1);
    if (ret_semop < 0) {
        perror("Error semop");
        exit(EXIT_FAILURE);
    }
}

void DumpSemaphores(int semid, size_t nsops)
{
    fprintf(stderr, "DumpSemaphores :\n");

    short* value_sems = (short*)calloc(nsops, sizeof(*value_sems));
    errno = 0;
    if (semctl(semid, 0, GETALL, value_sems) < 0) {
        perror("semctl()");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "SemVal = {");
    for (size_t i_sem = 0; i_sem < 5; i_sem++)
        fprintf(stderr, " %d", value_sems[i_sem]);
    fprintf(stderr, "}\n");

    free(value_sems);
}
// } Shell funcs
