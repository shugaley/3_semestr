
#include "shm_sem.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void WriteData(char* shmaddr, int semid);


void WriteFromSharedMemory()
{
    errno = 0;
    key_t key = ftok(FTOK_PATHNAME, FTOK_PROJ_ID);
    if (key < 0) {
        perror("Error ftok()");
        exit(EXIT_FAILURE);
    }

}


void WriteData(char* shmaddr, int semid)
{
}
