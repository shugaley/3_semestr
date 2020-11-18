
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
#include <sys/poll.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "proxy.h"

const int BASE_SIZE_BUFFER = 1024;

struct InfoLink {
    int    fd_writer;
    int    fd_reader;

    char*  buffer;
    char*  buffer_end;

    char*  cur_read;   //from buffer
    char*  cur_write;  //to buffer

    size_t size_empty;
    size_t size_full;
};


struct InfoChild {
    size_t numChild;
    pid_t  pid_child;

    int    fd_to_parent[2];
    int    fd_from_parent[2];
};

//TODO Delete = 0 after close

void ProxyChild (const char* path_input, struct InfoChild* infoChild, size_t nChilds);
void ProxyParent(struct InfoChild *infoChilds, size_t nChilds);

void MakeConnectionPipes(struct InfoChild *infoChild);
void CloseRedundantFdPipes_Parent(struct InfoChild *infoChild);
void CloseRedundantFdPipes_Child (struct InfoChild *infoChild);

size_t CountSizeBuffer(size_t base_size, size_t iChild, size_t nChild);

// Shell funcs {

pid_t Fork();

// } Shell funcs

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

        pid_t ret_fork = Fork();
        if (ret_fork == 0) {
            isChild = true;
            CloseRedundantFdPipes_Child(&infoChildCur);
            free(infoChilds);
        }

        if (!isChild) {
            infoChildCur.pid_child = ret_fork;
            CloseRedundantFdPipes_Parent(&infoChildCur);
            infoChilds[iChild] = infoChildCur;
        }
    }
    if (!isChild) {
        for (size_t i = 0; i < nChilds; i++) {
            printf("[%zu] :\n", i);
            DumpFd(&infoChilds[i]);
        }
    }

    if (isChild) {
    //    ProxyChild(path_input, &infoChildCur, nChilds);
        exit(EXIT_SUCCESS);
    }
    else
       // ProxyParent(infoChilds, nChilds);

    free(infoChilds);

    sleep(5);
}

//-----------------------------------------------------------------------------

void ProxyChild(const char* path_input, struct InfoChild* infoChild, size_t nChilds)
{
    assert(path_input);
    assert(infoChild);

    int ret = 0;

    //prepare fd
    int fd_reader = 0;
    int fd_writer = infoChild->fd_to_parent[1];

    if (infoChild->numChild == 0) {
        errno = 0;
        ret = close(infoChild->fd_from_parent[0]);
        if (ret < 0) {
            perror("Error close");
            exit(EXIT_FAILURE);
        }

        errno = 0;
        fd_reader = open (path_input, O_RDONLY);
        if (fd_reader < 0) {
            perror("Error open");
            exit(EXIT_FAILURE);
        }
    }
    else
        fd_reader = infoChild->fd_from_parent[0];

    //fcntl
    errno = 0;
    ret = fcntl(fd_reader, F_SETFL, O_RDONLY);
    if (ret < 0) {
        perror("Error fcntl");
        exit(EXIT_FAILURE);
    }

    errno = 0;
    ret = fcntl(fd_writer, F_SETFL, O_RDONLY);
    if (ret < 0) {
        perror("Error fcntl");
        exit(EXIT_FAILURE);
    }

    //splice
    ssize_t ret_splice = -1;
    while (ret_splice) {
        errno = 0;
        ret_splice = splice(fd_reader, NULL, fd_writer, NULL, PIPE_BUF, SPLICE_F_MOVE);
        if (ret_splice < 0) {
            perror("Error splice");
            exit(EXIT_FAILURE);
        }
    }

    //close
    errno = 0;
    ret = close(fd_writer);
    if (ret < 0) {
        perror("Error close");
        exit(EXIT_FAILURE);
    }

    errno = 0;
    ret = close(fd_reader);
    if (ret < 0) {
        perror("Error close");
        exit(EXIT_FAILURE);
    }
}


void ProxyParent(struct InfoChild *infoChilds, size_t nChilds)
{
    assert(infoChilds);

    size_t nLinks = nChilds - 1;

    struct InfoLink* IL = (struct InfoLink*)calloc(nLinks, sizeof(*IL));

    for (size_t i_link = 0; i_link < nChilds - 1; i_link++) {

        IL[i_link].fd_writer = infoChilds->fd_to_parent[0];
        IL[i_link].fd_reader = infoChilds->fd_from_parent[1];

        size_t size_buffer = CountSizeBuffer(BASE_SIZE_BUFFER, i_link, nChilds);
        IL[i_link].buffer = (char*)calloc(size_buffer, sizeof(*IL[i_link].buffer));
        IL[i_link].buffer_end = IL[i_link].buffer + size_buffer;

        IL[i_link].size_empty = size_buffer;
        IL[i_link].size_full  = 0;
    }

    int ret_pool_writers = -1;
    int ret_pool_readers = -1;
    while (ret_pool_writers) {
        //poll
        size_t nWriters = nChilds;
        size_t nReaders = nChilds - 1;

        struct pollfd* fdpoll_writers =
                (struct pollfd*)calloc(nWriters, sizeof(*fdpoll_writers));
        struct pollfd* fdpoll_readers =
                (struct pollfd*)calloc(nReaders, sizeof(*fdpoll_readers));

        for (size_t i_link = 0; i_link < nLinks; i_link++) {

            fdpoll_writers[i_link].fd = IL[i_link].fd_writer;
            fdpoll_writers[i_link].events = POLLIN;

            fdpoll_readers[i_link].fd     = IL[i_link].fd_reader;
            fdpoll_readers[i_link].events = POLLOUT;
        }

        errno = 0;
        ret_pool_writers = poll(fdpoll_writers, nWriters, 0);
        if (ret_pool_writers < 0) {
            perror("Error pool");
            exit(EXIT_FAILURE);
        }

        errno = 0;
        ret_pool_readers = poll(fdpoll_readers, nReaders, 0);
        if (ret_pool_readers < 0) {
            perror("Error pool");
            exit(EXIT_FAILURE);
        }

        //read & write
        for (size_t i_link = 0; i_link < nLinks; i_link++) {

            if (fdpoll_writers[i_link].revents == fdpoll_writers[i_link].events) {
            }

            if (fdpoll_readers[i_link].revents == fdpoll_readers[i_link].events &&
                IL[i_link].size_full > 0) {

                errno = 0;
                ssize_t ret_write = write(IL[i_link].fd_reader,
                                          IL[i_link].cur_read,
                                          IL[i_link].size_full);
                if (ret_write < 0) {
                    perror("Error write");
                    exit(EXIT_FAILURE);
                }

                if (IL[i_link].cur_read + ret_write == IL[i_link].buffer_end) {
                    IL[i_link].cur_read    = IL[i_link].buffer;
                    IL[i_link].size_empty += ret_write;
                    IL[i_link].size_full  -= ret_write;
                }
                else {
                    if (IL[i_link].cur_read > IL[i_link].cur_write)
                        ;
                }



            }



        }

    }

}

void MakeConnectionPipes(struct InfoChild *infoChild)
{
    assert(infoChild);

    int ret = 0;

    errno = 0;
    ret = pipe2(infoChild->fd_from_parent, O_NONBLOCK);
    if (ret < 0) {
        perror("Error pipe");
        exit(EXIT_FAILURE);
    }

    errno = 0;
    ret = pipe2(infoChild->fd_to_parent, O_NONBLOCK);
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

    if (infoChild->numChild == 0) {
        errno = 0;
        ret = close(infoChild->fd_from_parent[1]);
        if (ret < 0) {
            perror("Error close");
            exit(EXIT_FAILURE);
        }
        infoChild->fd_from_parent[1] = 0;
    }

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


size_t CountSizeBuffer(size_t base_size, size_t iChild, size_t nChild)
{
    size_t size = pow(3, (double)(nChild - iChild)) * 1024;
    return size;
}

// Shell funcs {

pid_t Fork()
{
    pid_t pid_parent = getpid();

    errno = 0;
    pid_t ret_fork = fork();
    if (ret_fork < -1) {
        perror("Error fork()");
        exit(EXIT_FAILURE);
    }

    if (ret_fork == 0) {
        int ret = prctl(PR_SET_PDEATHSIG, SIGTERM);
        if (ret < 0) {
            perror("Error ret()");
            //TODO kill childs
            exit(EXIT_FAILURE);
        }
        if (pid_parent != getppid()) {
            perror("Error parent process");
            exit(EXIT_FAILURE);
        }
    }

    return ret_fork;
}

// } Shell funcs

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
