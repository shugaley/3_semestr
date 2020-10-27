
#include "signal.h"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

void SendData(const char* path_input);
void GetData (pid_t pid_child);

void HandleOne(int num_signal, siginfo_t* siginfo, void* context);


void TransferDataToChild(const char* path_input)
{
    assert(path_input);

    pid_t pid_parent = getpid();
    pid_t pid_child = 0;
    errno = 0;
    switch (pid_child = fork()) {
    case -1:
        perror("Error fork()");
        exit(EXIT_FAILURE);

    case 0: {
        int ret_prctl = prctl(PR_SET_PDEATHSIG, SIGTERM);
        if (ret_prctl < 0) {
            perror("Error ret_prctl()");
            exit(EXIT_FAILURE);
        }
        if (pid_parent != getppid()) {
            perror("Error parent process");
            exit(EXIT_FAILURE);
        }
        SendData(path_input);
        exit(EXIT_SUCCESS);
    }

    default:
        GetData(pid_child);
        break;
    }
}


//-----------------------------------------------------------------------------

void SendData(const char* path_input)
{
    assert(path_input);
}


void GetData(pid_t pid_child)
{
}


void HandleOne(int num_signal, siginfo_t* siginfo, void* context)
{
    *((int*)context) = 1;
}
