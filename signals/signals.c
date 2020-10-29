
#include "signals.h"

#include <assert.h>
#include <errno.h>
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

bool current_bit = 0;

void SendData(const char* path_input);
void GetData (pid_t pid_child);

void HandleSendData (int num_signal);
void HandleEmpty    (int num_signal);
void HandleChildExit(int num_signal);

// Shell funcs {

void BlockSignals(int flag, const int* signals, size_t nsignals);

const int WITH_SIGNALS    = 1;
const int WITHOUT_SIGNALS = 2;
sigset_t CreateSigset(int flag, const int* signals, size_t nsignals);

struct sigaction CreateSigactionBase(void (*handler)(int));

void Sigaction(const struct SigactionUnion* sigactions, size_t nsigactions);
// } Shell funcs

//=============================================================================

void TransferDataFromChild(const char* path_input)
{
    assert(path_input);

    //block signals which use
    int signals_blocking[] = {SIGUSR1, SIGUSR2};
    size_t nsignals_blocking = sizeof(signals_blocking) /
                               sizeof(*signals_blocking);
    BlockSignals(SIG_BLOCK, signals_blocking, nsignals_blocking);

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

    pid_t pid_parent = getppid();

//    errno = 0;
//    FILE* input = fopen(path_input, "r");
//    if (input < 0) {
//        perror("Error fopen()");
//        exit(EXIT_FAILURE);
//    }
//
//    char input_char = 0;
//    errno = 0;
//    while ((input_char = (char)fgetc(input) != EOF))
//
//        for (size_t i_bit = 0; i_bit < sizeof(char) * 8; i_bit++) {
//            char mask = 0b01 << i_bit;
//
//            int ret_kill = 0;
//            errno = 0;
//            if (mask & input_char)
//                ret_kill = kill(pid_parent, SIGUSR1);
//            else
//                ret_kill = kill(pid_parent, SIGUSR2);
//
//            if (ret_kill < 0) {
//                perror("Error kill()");
//                exit(EXIT_FAILURE);
//            }
//        }
//
//    if (errno != 0) {
//        perror("Error fgetc()");
//        exit(EXIT_FAILURE);
//    }



//    printf("Child kill\n");
//    kill(pid_parent, SIGUSR1);

//    fclose(input);
}


void GetData(pid_t pid_child)
{
    struct sigaction sa_SendData = CreateSigactionBase(HandleSendData);

    struct SigactionUnion sigactions[] = {SIGUSR1, &sa_SendData, NULL,
                                          SIGUSR2, &sa_SendData, NULL};
    size_t nsigactions = sizeof(sigactions) /
                         sizeof(*sigactions);
    Sigaction(sigactions, nsigactions);

    int signals_get[] = {SIGUSR1, SIGUSR2};
    size_t nsignals_get = sizeof(signals_get) / sizeof(*signals_get);
    sigset_t sigset_get = CreateSigset(WITHOUT_SIGNALS, signals_get, nsignals_get);

    sigsuspend(&sigset_get);
}



void HandleSendData (int num_signal)
{
    if (num_signal == SIGUSR1) {
        return;
    }

    if (num_signal == SIGUSR2) {
        return;
    }
}

// Shell funcs {

void BlockSignals(int flag, const int* signals, size_t nsignals)
{
    assert(signals);

    int ret = 0;

    sigset_t sigset = {};
    errno = 0;
    ret = sigemptyset(&sigset);
    if (ret < 0) {
        perror("Error sigemptyset()");
        exit(EXIT_FAILURE);
    }

    for (size_t i_signal = 0; i_signal < nsignals; i_signal++) {
        errno = 0;
        ret = sigaddset(&sigset, signals[i_signal]);
        if (ret < 0) {
            perror("Error sigaddset()");
            exit(EXIT_FAILURE);
        }
    }

    errno = 0;
    ret = sigprocmask(flag, &sigset, NULL);
    if (ret < 0) {
        perror("Error sigprocmask()");
        exit(EXIT_FAILURE);
    }

}


sigset_t CreateSigset(int flag, const int* signals, size_t nsignals)
{
    assert(signals);

    int ret = 0;

    sigset_t sigset_get;
    errno = 0;
    ret = sigfillset(&sigset_get);
    if (ret < 0) {
        perror("Error sigfillset()");
        exit(EXIT_FAILURE);
    }

    for (size_t i_signal = 0; i_signal < nsignals; i_signal++) {
        errno = 0;
        ret = sigdelset(&sigset_get, signals[i_signal]);
        if (ret < 0) {
            perror("Error sigdelset()");
            exit(EXIT_FAILURE);
        }
    }

    return sigset_get;
}


struct sigaction CreateSigactionBase(void (*handler)(int))
{
    assert(handler);

    struct sigaction sa = {};
    errno = 0;
    int ret = sigfillset(&sa.sa_mask);
    if (ret < 0) {
        perror("Error sigfillset()");
        exit(EXIT_FAILURE);
    }
    sa.sa_handler = handler;
    //sa_SendData.sa_flags = SA_NODEFER;

    return sa;
}


void Sigaction(const struct SigactionUnion* sigactions, size_t nsigactions)
{
    assert(sigactions);

    for (size_t i_sa = 0; i_sa < nsigactions; i_sa++) {
        errno = 0;
        int ret = sigaction(sigactions->num_signal, sigactions->sa,
                                                    sigactions->sa_old);
        if (ret < 0) {
            perror("Error sigaction()");
            exit(EXIT_FAILURE);
        }
    }
}

// } Shell funcs
