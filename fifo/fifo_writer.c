
#include "fifo.h"

pid_t ReadFifo_Pid(const char* pathFile_FifoPid);

//=============================================================================

void WriteFifo(const char* pathFile_Input)
{
    assert(pathFile_Input);

    pid_t pid_FifoReader = ReadFifo_Pid(PATH_FILE_FIFO_TRANSFER_PID);

    char* pathFifo = MakePathFifo(pid_FifoReader);
    printf("pathFifo(writer): %s\n", pathFifo); //TODO FOR DEBUG
}


pid_t ReadFifo_Pid(const char* pathFile_FifoPid)
{
    assert(pathFile_FifoPid);

    Mkfifo (pathFile_FifoPid, MKFIFO_MODE_DEFAULT,
            "Error mkfifo FifoPid(writer)");

    int fd_FifoPid = Open(pathFile_FifoPid, O_RDONLY,
                          "Error open FifoPid(writer)");

    pid_t pid = 0;
    Read(fd_FifoPid, &pid, sizeof(pid_t), "Error read pid_writer");

    return pid;
}
