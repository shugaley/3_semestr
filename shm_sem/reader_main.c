
#include "shm_sem.h"

#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static const size_t N_KEYS  = 2;

char* ScanKeys(int argc, const char** argv);


int main(const int argc, const char** argv)
{
    char* path_input = ScanKeys(argc, argv);

    ReadToSharedMemory(path_input);

    return 0;
}


//Scan Keys {

char* ScanKeys(int argc, const char** argv)
{
    if(argc != N_KEYS) {
        perror("Wrong nKeys");
        exit(EXIT_FAILURE);
    }

    char* buffer = (char*)calloc(strlen(argv[1]), sizeof(buffer));

    strcpy(buffer, argv[1]);
    return buffer;
}

// } Scan Keys
