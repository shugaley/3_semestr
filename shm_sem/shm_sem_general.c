
#include "shm_sem_general.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

// Shell funcs {


char* ConstructSharedMemory(key_t key, size_t size, int* shmid)
{
    errno = 0;
    int ret_shmget = shmget(key, size, 0666 | IPC_CREAT);
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


void AssignSem(int id_sem, int num_sem, short value)
{
    int ret = semctl(id_sem, num_sem, SETVAL, value);
    if (ret < 0) {
        perror("Error semctl");
        exit(EXIT_FAILURE);
    }
}


void DumpSemaphores(int semid, size_t nsops, const char* str)
{
    fprintf(stderr, "DumpSemaphores(%s) : ", str);

    short* value_sems = (short*)calloc(nsops, sizeof(*value_sems));
    errno = 0;
    if (semctl(semid, 0, GETALL, value_sems) < 0) {
        perror("semctl()");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "SemVal = {");
    for (size_t i_sem = 0; i_sem < nsops; i_sem++)
        fprintf(stderr, " %d", value_sems[i_sem]);
    fprintf(stderr, "}\n");

    free(value_sems);
}
// } Shell funcs
