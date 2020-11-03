
#include "signals.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>


struct SigactionUnion {
    int num_signal;
    struct sigaction* sa;
    struct sigaction* sa_old;
};

volatile bool current_bit  = 0;

void SendData(const char* path_input);
void GetData (pid_t pid_child);

void HandleGetData(int num_signal);
void HandleEmpty       (int num_signal);
void HandleSigchld     (int num_signal);

// Shell funcs {

const bool WITH_SIGNALS    = 1;
const bool WITHOUT_SIGNALS = 0;
sigset_t CreateSigset(const int* signals, size_t nsignals, bool isWithSignals);

// } Shell funcs

//=============================================================================

void TransferDataFromChild(const char* path_input)
{
    assert(path_input);

    //block signals which use
    int signals_block[] = {SIGUSR1, SIGUSR2};
    size_t nsignals_block = sizeof( signals_block) / sizeof(*signals_block);
    sigset_t sigset_block = CreateSigset(signals_block, nsignals_block, WITH_SIGNALS);

    errno = 0;
    int ret = sigprocmask(SIG_BLOCK, &sigset_block, NULL);
    if (ret < 0) {
        perror("Error sigprocmask()");
        exit(EXIT_FAILURE);
    }

    pid_t pid_parent = getpid();
    pid_t pid_child = 0;
    errno = 0;
    switch (pid_child = fork()) {
    case -1:
        perror("Error fork()");
        exit(EXIT_FAILURE);

    case 0: {
        ret = prctl(PR_SET_PDEATHSIG, SIGTERM);
        if (ret < 0) {
            perror("Error ret()");
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

    struct sigaction sa_empty = {};
    errno = 0;
    int ret = sigfillset(&sa_empty.sa_mask);
    if (ret < 0) {
        perror("Error sigfillset()");
        exit(EXIT_FAILURE);
    }
    sa_empty.sa_handler = HandleEmpty;
    sa_empty.sa_flags = SA_NODEFER;

    errno = 0;
    ret = sigaction(SIGUSR1, &sa_empty, NULL);
    if (ret < 0) {
        perror("Error sigaction()");
        exit(EXIT_FAILURE);
    }

    int signals_send[] = {SIGUSR1, SIGTERM, SIGINT};
    size_t nsignals_send = sizeof(signals_send) / sizeof(*signals_send);
    sigset_t sigset_send = CreateSigset(signals_send, nsignals_send, WITHOUT_SIGNALS);

    errno = 0;
    FILE* input = fopen(path_input, "r");
    if (input < 0) {
        perror("Error fopen()");
        exit(EXIT_FAILURE);
    }

    char input_char = 0;
    errno = 0;
    while ((input_char = (char)fgetc(input)) != EOF)

        for (size_t i_bit = 0; i_bit < CHAR_BIT; i_bit++) {
            char current_mask = 0b01 << i_bit;

            pid_t pid_parent = getppid();

            int ret_kill = 0;
            errno = 0;
            if (current_mask & input_char)
                ret_kill = kill(pid_parent, SIGUSR1);
            else
                ret_kill = kill(pid_parent, SIGUSR2);
            if (ret_kill < 0) {
                perror("Error kill()");
                exit(EXIT_FAILURE);
            }

            //printf("child before suspend\n");
            errno = 0;
            sigsuspend(&sigset_send);
            if (errno != EINTR) {
                perror("Error sigsuspend()");
                exit(EXIT_FAILURE);
            }
            errno = 0;
            //printf("child after suspend\n");
        }
    if (errno != 0) {
        perror("Error fgetc()");
        exit(EXIT_FAILURE);
    }

//    printf("Child kill\n");
//    kill(getppid(), SIGUSR1);

    fclose(input);
}


void GetData(pid_t pid_child)
{
    struct sigaction sa_GetData = {};
    errno = 0;
    int ret = sigfillset(&sa_GetData.sa_mask);
    if (ret < 0) {
        perror("Error sigfillset()");
        exit(EXIT_FAILURE);
    }
    sa_GetData.sa_handler = HandleGetData;
    sa_GetData.sa_flags = SA_NODEFER;

    errno = 0;
    ret = sigaction(SIGUSR1, &sa_GetData, NULL);
    if (ret < 0) {
        perror("Error sigaction()");
        exit(EXIT_FAILURE);
    }
    errno = 0;
    ret = sigaction(SIGUSR2, &sa_GetData, NULL);
    if (ret < 0) {
        perror("Error sigaction()");
        exit(EXIT_FAILURE);
    }

    int signals_get[] = {SIGUSR1, SIGUSR2, SIGCHLD, SIGTERM, SIGINT};
    size_t nsignals_get = sizeof(signals_get) / sizeof(*signals_get);
    sigset_t sigset_get = CreateSigset(signals_get, nsignals_get, WITHOUT_SIGNALS);

    while (1) {
        char output_char = 0;
        for (size_t i_bit = 0; i_bit < CHAR_BIT; i_bit++) {
            char current_mask = 0b01 << i_bit;

            errno = 0;
            //printf("parent sigsuspend\n");
            sigsuspend(&sigset_get);
            if (errno != EINTR) {
                perror("Error sigsuspend()");
                exit(EXIT_FAILURE);
            }

            //printf("parent kill\n");
            if (current_bit == 1)
                output_char = output_char | current_mask;

            errno = 0;
            ret = kill(pid_child, SIGUSR1);
            if (ret < 0) {
                perror("Error kill()");
                exit(EXIT_FAILURE);
            }
        }
        printf("%c", output_char);
        fflush(stdout);
    }
}



void HandleGetData(int num_signal)
{
    if (num_signal == SIGUSR1)
        current_bit = 1;

    if (num_signal == SIGUSR2)
        current_bit = 0;
}

void HandleEmpty(int num_signal)
{
    return;
}

void HandleSigchld(int num_signal)
{

}

// Shell funcs {

sigset_t CreateSigset(const int* signals, size_t nsignals, bool isWithSignals)
{
    assert(signals);

    int ret = 0;

    sigset_t sigset = {};
    errno = 0;
    if (isWithSignals)
        ret = sigemptyset(&sigset);
    else
        ret = sigfillset(&sigset);
    if (ret < 0) {
        perror("Error sigfillset()");
        exit(EXIT_FAILURE);
    }

    for (size_t i_signal = 0; i_signal < nsignals; i_signal++) {
        errno = 0;
        if (isWithSignals)
            ret = sigaddset(&sigset, signals[i_signal]);
        else
            ret = sigdelset(&sigset, signals[i_signal]);
        if (ret < 0) {
            perror("Error sigdelset()");
            exit(EXIT_FAILURE);
        }
    }

    return sigset;
}

// } Shell funcs
