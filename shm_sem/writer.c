
#include "shm_sem.h"

#include "assert.h"
#include "stdio.h"
#include "sys/types.h"
#include "sys/ipc.h"

void WriteSharedMemory(const char* pathInput)
{
    assert(pathInput);

    key_t key = ftok(FTOK_PATHNAME, FTOK_PROJ_ID);


}
