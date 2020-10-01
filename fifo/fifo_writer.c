
#include "fifo.h"

pid_t ReadFifoPid(const char* pathFifoPid);

//=============================================================================

void WriteFifo(const char* pathInput)
{
    assert(pathInput);

    pid_t pid_FifoReader = ReadFifoPid(PATH_FILE_FIFO_TRANSFER_PID);

    char* pathFifo = MakePathFifo(pid_FifoReader);

    int fd_Fifo = Open(pathFifo, O_WRONLY | O_NONBLOCK,
                       "Error open pathFifo(writer)\n");
    Fcntl(fd_Fifo, F_SETFL, O_WRONLY, "Error fcntrl(pathFifo)(writer)\n");

    int fd_Input = Open(pathInput, O_RDONLY, "Error open FileInput\n");

    int    ret_splice = 0;
    while((ret_splice = splice(fd_Input, NULL, fd_Fifo,
                            NULL, PIPE_BUF, SPLICE_F_MOVE))) {
        if(ret_splice < 0) {
            perror("Error splice(writer)\n");
            exit(EXIT_FAILURE);
        }
    }

    close(fd_Fifo);
    unlink(pathFifo);

    close(fd_Input);
}


pid_t ReadFifoPid(const char* pathFifoPid)
{
    assert(pathFifoPid);

    Mkfifo(pathFifoPid, MKFIFO_MODE_DEFAULT,
           "Error mkfifo FifoPid(writer)");

    int fd_FifoPid = Open(pathFifoPid, O_RDONLY,
                          "Error open FifoPid(writer)");

    pid_t pid = 0;
    Read(fd_FifoPid, &pid, sizeof(pid_t), "Error read pid_writer");

    return pid;
}
