
#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "proxy.h"


struct InfoConnection {
    int    fd_reader;
    int    fd_writer;

    char*  buffer;
    size_t size_buffer;
    char*  buffer_end;

    char*  buffer_cur_read;
    char*  buffer_cur_write;

    size_t size_empty;
    size_t size_full;
};


struct InfoChild {
    size_t numChild;
    pid_t  pid_child;

    int    fd_to_parent[2];
    int    fd_from_parent[2];
};

//TODO
//Think about close with close

void ProxyChild (const char* path_input, struct InfoChild* infoChild, size_t nChilds);
void ProxyParent(struct InfoChild *infoChilds, size_t nChilds);

void MakeConnectionPipes(struct InfoChild *infoChild);
void CloseRedundantFdPipes_Parent(struct InfoChild *infoChild);
void CloseRedundantFdPipes_Child (struct InfoChild *infoChild);

size_t CountSizeBuffer(size_t maxsize, size_t iChild, size_t nChild);

void DumpFd(struct InfoChild *infoChild);

//-----------------------------------------------------------------------------

void ProxyChilds(const char* path_input, size_t nChilds)
{
    struct InfoChild* infoChilds = (struct InfoChild*)calloc(nChilds * 2,
                                                             sizeof(*infoChilds));
    bool isChild = false;
    struct InfoChild infoChildCur = {};
    for (size_t iChild = 0; iChild < nChilds && !isChild; iChild++) {
        int ret = 0;

        infoChildCur.numChild  = iChild;
        MakeConnectionPipes(&infoChildCur);

        //fork()
        pid_t pid_parent = getpid();
        pid_t ret_fork = 0;

        errno = 0;
        switch (ret_fork = fork()) {
        case -1:
            perror("Error fork()");
            exit(EXIT_FAILURE);

        case 0:
            ret = prctl(PR_SET_PDEATHSIG, SIGTERM);
            if (ret < 0) {
                perror("Error ret()");
                exit(EXIT_FAILURE);
            }
            if (pid_parent != getppid()) {
                perror("Error parent process");
                exit(EXIT_FAILURE);
            }
            isChild = true;
            CloseRedundantFdPipes_Child(&infoChildCur);
            break;
        default:
            infoChildCur.pid_child = ret_fork;
            CloseRedundantFdPipes_Parent(&infoChildCur);
            infoChilds[iChild] = infoChildCur;
        }
    }

    if (isChild) {
        free(infoChilds);
        ProxyChild(path_input, &infoChildCur, nChilds);
        exit(EXIT_SUCCESS);
    }
    else
        ProxyParent(infoChilds, nChilds);

    free(infoChilds);

    sleep(5);
}

//-----------------------------------------------------------------------------

void ProxyChild(const char* path_input, struct InfoChild* infoChild, size_t nChilds)
{
    assert(path_input);
    assert(infoChild);

    return;
    //TODO fcntl, open file

    int fd_reader = infoChild->fd_from_parent[0];
    int fd_writer = infoChild->fd_to_parent[1];

    ssize_t ret_splice = 0;
    while( (ret_splice = splice(fd_reader, NULL,
                                fd_writer, NULL, PIPE_BUF, SPLICE_F_MOVE)) ) {


    }
}


void ProxyParent(struct InfoChild *infoChilds, size_t nChilds)
{
    assert(infoChilds);


}

void MakeConnectionPipes(struct InfoChild *infoChild)
{
    assert(infoChild);

    int ret = 0;

    errno = 0;
    ret = pipe(infoChild->fd_from_parent);
    if (ret < 0) {
        perror("Error pipe");
        exit(EXIT_FAILURE);
    }

    errno = 0;
    ret = pipe(infoChild->fd_to_parent);
    if (ret < 0) {
        perror("Error pipe");
        exit(EXIT_FAILURE);
    }
}


void CloseRedundantFdPipes_Parent(struct InfoChild *infoChild)
{
    assert(infoChild);

    int ret = 0;

    errno  = 0;
    ret = close(infoChild->fd_from_parent[0]);
    if (ret < 0) {
        perror("Error close");
        exit(EXIT_FAILURE);
    }
    infoChild->fd_from_parent[0] = 0;

    errno = 0;
    ret = close(infoChild->fd_to_parent[1]);
    if (ret < 0) {
        perror("Error close");
        exit(EXIT_FAILURE);
    }
    infoChild->fd_to_parent[1] = 0;
}


void CloseRedundantFdPipes_Child(struct InfoChild *infoChild)
{
    assert(infoChild);

    int ret = 0;

    errno = 0;
    ret = close(infoChild->fd_from_parent[1]);
    if (ret < 0) {
        perror("Error close");
        exit(EXIT_FAILURE);
    }
    infoChild->fd_from_parent[1] = 0;

    errno = 0;
    ret = close(infoChild->fd_to_parent[0]);
    if (ret < 0) {
        perror("Error close");
        exit(EXIT_FAILURE);
    }
    infoChild->fd_to_parent[0] = 0;
}


size_t CountSizeBuffer(size_t maxsize, size_t iChild, size_t nChild)
{
    size_t size = pow(3, (double)(nChild - iChild)) * 1024;
    return size;
}


void DumpFd(struct InfoChild *infoChild)
{
    assert(infoChild);

    char* strDump = (char*)calloc(1000, sizeof(*strDump));

    sprintf(strDump + strlen(strDump),
            "[%zu]fd_to_parent[0]   - %d\n", infoChild->numChild,
                                             infoChild->fd_to_parent[0]);
    sprintf(strDump + strlen(strDump),
            "[%zu]fd_to_parent[1]   - %d\n", infoChild->numChild,
                                             infoChild->fd_to_parent[1]);
    sprintf(strDump + strlen(strDump),
            "[%zu]fd_from_parent[0] - %d\n", infoChild->numChild,
                                             infoChild->fd_from_parent[0]);
    sprintf(strDump + strlen(strDump),
            "[%zu]fd_from_parent[1] - %d\n", infoChild->numChild,
                                             infoChild->fd_from_parent[1]);
    sprintf(strDump + strlen(strDump),
            "\n\n");

    printf("%s\n", strDump);
}
