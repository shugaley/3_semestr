
#include "fifo.h"

static const size_t N_KEYS  = 2;

char* ScanKeys(const int argc, const char** argv);

//=============================================================================


int main(const int argc, const char** argv)
{
    char* pathFile_Input = ScanKeys(argc, argv);

    WriteFifo(pathFile_Input);

    return 0;
}


//Scan Keys {

char* ScanKeys(const int argc, const char** argv)
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