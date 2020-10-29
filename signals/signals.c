
#include "signals.h"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>


struct Sigaction_t {
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

//const int WITH_SIGNALS    = 1;
const int WITHOUT_SIGNALS = 2;
sigset_t CreateSigset(int flag, const int* signals, size_t nsignals);

void Sigaction(const struct Sigaction_t* sigactions, size_t nsigactions);
// } Shell funcs

//=============================================================================

void TransferDataFromChild(const char* path_input)
{
    assert(path_input);

    //block signals which use
    int signals_blocking[] = {SIGUSR1, SIGUSR2};
    BlockSignals(SIG_BLOCK, signals_blocking, sizeof(signals_blocking));

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
    int ret = 0;

    struct sigaction sa_sigusr = {};
    errno = 0;
    ret = sigfillset(&sa_sigusr.sa_mask);
    if (ret < 0) {
        perror("Error sigfillset()");
        exit(EXIT_FAILURE);
    }
    sa_sigusr.sa_handler = HandleSendData;
    //sa_sigusr.sa_flags = SA_NODEFER;

    struct Sigaction_t sigactions[] = {SIGUSR1, &sa_sigusr, NULL,
                                       SIGUSR2, &sa_sigusr, NULL};
    Sigaction(sigactions, sizeof(sigactions));

    int signals_get[] = {SIGUSR1, SIGUSR2};
    sigset_t sigset_get = CreateSigset(WITHOUT_SIGNALS, signals_get,
                                       sizeof(signals_get));

    //sigsuspend(&sigset_get);
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

void Sigaction(const struct Sigaction_t* sigactions, size_t nsigactions)
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
