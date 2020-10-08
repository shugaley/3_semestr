
#include "fifo.h"

void WriteFifo_Pid(const pid_t pid, const char* pathFile_FifoPid);

//=============================================================================

void ReadFifo()
{
    pid_t pid_FifoReader = getpid();

    char* pathFifo = MakePathFifo(pid_FifoReader);

    Mkfifo(pathFifo, MKFIFO_MODE_DEFAULT, "Error mkfifo fifo");
    int fd_Fifo = Open(pathFifo, O_RDONLY | O_NONBLOCK,
                       "Error open pathFifo(reader)");

    WriteFifo_Pid(pid_FifoReader, PATH_FILE_FIFO_TRANSFER_PID);

    if (!IsCanReadFile(fd_Fifo)) {
        perror("Error can't connect (time out) (reader)");
        exit(EXIT_FAILURE);
    }

    Fcntl(fd_Fifo, F_SETFL, O_RDONLY & ~O_RDWR, "Error fcntl fifo(reader)");

    if(isatty(STDOUT_FILENO))
        fcntl(STDOUT_FILENO, F_SETFL, fcntl(STDOUT_FILENO, F_GETFL) & ~O_APPEND);

    int ret_splice = 0;
    while ((ret_splice = splice(fd_Fifo, NULL, STDOUT_FILENO, NULL,
                                10, SPLICE_F_MOVE))) {
        printf("  ret splice %d\n", ret_splice);
        if (ret_splice < 0) {
            perror("Error splice(reader)\n");
            exit(EXIT_FAILURE);
        }
    }

    close(fd_Fifo);
    unlink(pathFifo);
}

//-----------------------------------------------------------------------------

void WriteFifo_Pid(const pid_t pid, const char* pathFile_FifoPid)
{
    assert(pathFile_FifoPid);

    Mkfifo(pathFile_FifoPid, MKFIFO_MODE_DEFAULT,
           "Error mkfifo FifoPid(reader)");

    int fd_FifoPid = Open(pathFile_FifoPid, O_WRONLY,
                          "Error open FifoPid(reader)");

    Write(fd_FifoPid, &pid, sizeof(pid_t), "Error write pid");

    close(fd_FifoPid);
}
