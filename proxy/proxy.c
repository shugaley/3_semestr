
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "proxy.h"


struct InfoConnection {
    int    fr_writer;
    int    fd_reader;

    char*  buffer;
    char*  buffer_end;
    size_t size_buffer;

    char*  buffer_cur_write;
    char*  buffer_cur_read;

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
void ProxyParent(struct InfoChild* infoChild);

void MakeConnectionPipes(struct InfoChild* infoChild, size_t nChilds);
void CloseRedundantPipes(bool isChild, struct InfoChild* infoChild);

size_t CountSizeBuffer(size_t maxsize, size_t iChild, size_t nChild);

void DumpFd(bool isChild, struct InfoChild* infoChilds, size_t nChild);

//-----------------------------------------------------------------------------

void ProxyChilds(const char* path_input, size_t nChilds)
{
    struct InfoChild* infoChilds = (struct InfoChild*)calloc(nChilds * 2,
                                                             sizeof(*infoChilds));
    bool isChild = false;
    size_t numChild = 0;
    for (size_t iChild = 0; iChild < nChilds && !isChild; iChild++) {
        int ret = 0;

        numChild = iChild;
        infoChilds[iChild].numChild  = numChild;
        MakeConnectionPipes(&infoChilds[iChild], nChilds);

        //fork()
        pid_t pid_child = 0;
        pid_t pid_parent = getpid();

        errno = 0;
        switch (pid_child = fork()) {
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
            break;
        default:
            infoChilds[iChild].pid_child = pid_child;
        }

       // CloseRedundantPipes(isChild, &infoChilds[iChild]);
    }

    if (isChild)
        ProxyChild(path_input, &infoChilds[numChild], nChilds);
    else
        ProxyParent(&infoChilds[numChild]);

    free(infoChilds);

}

//-----------------------------------------------------------------------------


void ProxyChild(const char* path_input, struct InfoChild* infoChild, size_t nChilds)
{
    assert(path_input);
    assert(infoChild);

    //TODO fcntl, open file



}


void ProxyParent(struct InfoChild* infoChild)
{
    assert(infoChild);


}

void MakeConnectionPipes(struct InfoChild* infoChild, size_t nChilds)
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

    if (infoChild->numChild == 0) {
        errno = 0;
        ret = close(infoChild->fd_from_parent[0]);
        if (ret < 0) {
            perror("Error close");
            exit(EXIT_FAILURE);
        }
    }

    if (infoChild->numChild == nChilds - 1) {
        errno = 0;
        ret = close(infoChild->fd_to_parent[1]);
        if (ret < 0) {
            perror("Error close");
            exit(EXIT_FAILURE);
        }
    }
}


void CloseRedundantPipes (bool isChild, struct InfoChild* infoChild)
{
    assert(infoChild);

    int ret = 0;
    bool isParent = !isChild;

    if (isParent) {
        errno  = 0;
        ret = close(infoChild->fd_from_parent[0]);
        if (ret < 0) {
            perror("Error close");
            exit(EXIT_FAILURE);
        }

        errno = 0;
        ret = close(infoChild->fd_to_parent[1]);
        if (ret < 0) {
            perror("Error close");
            exit(EXIT_FAILURE);
        }
    }

    if (isChild) {
        errno = 0;
        ret = close(infoChild->fd_from_parent[1]);
        if (ret < 0) {
            perror("Error close");
            exit(EXIT_FAILURE);
        }

        errno = 0;
        ret = close(infoChild->fd_to_parent[0]);
        if (ret < 0) {
            perror("Error close");
            exit(EXIT_FAILURE);
        }
    }
}


size_t CountSizeBuffer(size_t maxsize, size_t iChild, size_t nChild)
{
    size_t size = pow(3, (double)(nChild - iChild)) * 1024;
    return size;
}


void DumpFd(bool isChild, struct  InfoChild* infoChilds, size_t nChild)
{
    assert(infoChilds);

    if (isChild)
        printf("CHILD :\n");
    else
        printf("Parent :\n");

    for (size_t i_infoChild = 0; i_infoChild < nChild; i_infoChild++) {

        printf("[%zu] ", i_infoChild);
        //printf("", infoChilds[i_infoChild].)
    }
}
