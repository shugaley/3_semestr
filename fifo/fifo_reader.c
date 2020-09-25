
#include "fifo.h"

static const time_t TIME_WAITING = 1;

void WriteFifo_Pid(const pid_t pid, const char* pathFile_FifoPid);

//=============================================================================

void ReadFifo()
{
    pid_t pid_FifoReader = getpid();

    char* pathFifo = MakePathFifo(pid_FifoReader);
    printf("pathFifo(reader): %s\n", pathFifo); //TODO FOR DEBUG

    Mkfifo(pathFifo, MKFIFO_MODE_DEFAULT,   "Error mkfifo fifo");
    int fd_Fifo = Open(pathFifo, O_RDONLY | O_NONBLOCK,
                       "Error open pathFifo(reader)");

    WriteFifo_Pid(pid_FifoReader, PATH_FILE_FIFO_TRANSFER_PID);

    struct timeval tv_Fifo;
    tv_Fifo.tv_sec  = TIME_WAITING;
    tv_Fifo.tv_usec = 0;

    Fcntl(fd_Fifo, F_SETFL, O_RDONLY, "Error fcntl fifo(reader)");

}


void WriteFifo_Pid(const pid_t pid, const char* pathFile_FifoPid)
{
    assert(pathFile_FifoPid);

    Mkfifo (pathFile_FifoPid, MKFIFO_MODE_DEFAULT,
            "Error mkfifo FifoPid(reader)");

    int fd_FifoPid = Open (pathFile_FifoPid, O_WRONLY,
                           "Error open FifoPid(reader)");

    Write(fd_FifoPid, &pid, sizeof(pid_t), "Error write pid");
}
