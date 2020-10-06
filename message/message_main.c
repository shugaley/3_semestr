
#include "message.h"

static const size_t N_KEYS  = 2;

size_t ScanKeys(const int argc, const char** argv);

//=============================================================================

int main(int argc, char** argv)
{
    size_t nProcess = ScanKeys(argc, argv);

    Print_NumChildProcesses(nProcess);

    return 0;
}


size_t ScanKeys(const int argc, const char** argv)
{
    assert(argv);

    if(argc != N_KEYS) {
        perror("Wrong nKeys");
        exit(EXIT_FAILURE);
    }

    size_t number = ScanKeyNumber(argv[1]);
    return number;
}
