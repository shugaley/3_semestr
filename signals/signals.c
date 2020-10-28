
#include "signals.h"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#define SIGNALS_USED 1

void SendData(const char* path_input);
void GetData (pid_t pid_child);

void HandleBitZero(int num_signal);
void HandleBitOne (int num_signal);

void BlockSignals(int flag, const int* signals, size_t nsignals);

//=============================================================================

void TransferDataFromChild(const char* path_input)
{
    assert(path_input);

    //block signals which use

    int signals_blocking[SIGNALS_USED] = {SIGUSR1};

    BlockSignals(SIG_BLOCK, signals_blocking, SIGNALS_USED);

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

    sleep(5);

    pid_t pid_parent = getppid();
    printf("Child kill\n");
    kill(pid_parent, SIGUSR1);
}


void GetData(pid_t pid_child)
{
    struct sigaction sa_one = {};
    errno = 0;
    int ret = sigfillset(&sa_one.sa_mask);
    if (ret < 0) {
        perror("Error sigfillset()");
        exit(EXIT_FAILURE);
    }
    sa_one.sa_handler = HandleBitOne;
    sa_one.sa_flags = SA_NODEFER;

    errno = 0;
    ret = sigaction(SIGUSR1, &sa_one, NULL);
    if (ret < 0) {
        perror("Error sigaction()");
        exit(EXIT_FAILURE);
    }

    sigset_t sigset_parent;
    errno = 0;
    ret = sigfillset(&sigset_parent);
    if (ret < 0) {
        perror("Error sigfillset()");
        exit(EXIT_FAILURE);
    }

    errno = 0;
    ret = sigdelset(&sigset_parent, SIGUSR1);
    if (ret < 0) {
        perror("Error sigdelset()");
        exit(EXIT_FAILURE);
    }

    sigsuspend(&sigset_parent);

    printf("Parent ready\n");

    while(1);
}


void HandleBitOne(int num_signal)
{
    printf("SSSSSSSUCK\n");
}



void BlockSignals(int flag, const int* signals, size_t nsignals)
{
    assert(signals);

    sigset_t sigset_base = {};
    errno = 0;
    int ret = sigemptyset(&sigset_base);
    if (ret < 0) {
        perror("Error sigemptyset()");
        exit(EXIT_FAILURE);
    }

    for (size_t i_signal = 0; i_signal < nsignals; i_signal++) {
        ret = sigaddset(&sigset_base, signals[i_signal]);
        if (ret < 0) {
            perror("Error sigaddset()");
            exit(EXIT_FAILURE);
        }
    }

    errno = 0;
    ret = sigprocmask(SIG_BLOCK, &sigset_base, NULL);
    if (ret < 0) {
        perror("Error sigprocmask()");
        exit(EXIT_FAILURE);
    }

}
