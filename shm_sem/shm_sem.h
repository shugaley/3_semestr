#ifndef SHM_SEM_SHM_SEM_H
#define SHM_SEM_SHM_SEM_H

#include <sys/types.h>

static const char FTOK_PATHNAME[] = __FILE__;
static const int  FTOK_PROJ_ID    = 0;

void WriteSharedMemory(const char* pathInput);
void ReadSharedMemory ();

#endif //SHM_SEM_SHM_SEM_H
