
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

    bool isWasEOF;
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

void  WriteToBuffer(struct InfoLink* IL);
void ReadFromBuffer(struct InfoLink* IL);

size_t CountSizeBuffer(size_t base_size, size_t iChild, size_t nChild);

// Work with pipe {
void MakeConnectionPipes         (struct InfoChild *infoChild);
void CloseRedundantFdPipes_Parent(struct InfoChild *infoChild);
void CloseRedundantFdPipes_Child (struct InfoChild *infoChild);
// } Work with pipe

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

        infoChildCur.numChild  = iChild;
        MakeConnectionPipes(&infoChildCur);

        pid_t ret_fork = Fork();
        if (ret_fork == 0) {
            isChild = true;
            for (size_t jChild = 0; jChild < infoChildCur.numChild; jChild++)
                CloseRedundantFdPipes_Child(&infoChilds[jChild]);
            free(infoChilds);
            CloseRedundantFdPipes_Child(&infoChildCur);
        }

        if (!isChild) {
            infoChildCur.pid_child = ret_fork;
            CloseRedundantFdPipes_Parent(&infoChildCur);
            infoChilds[iChild] = infoChildCur;
        }
    }

    if (isChild) {
        ProxyChild(path_input, &infoChildCur, nChilds);
        exit(EXIT_SUCCESS);
    }
    else {
        errno = 0;
        int ret = close(infoChilds[0].fd_from_parent[1]);
        if (ret < 0) {
            perror("Error close");
            exit(EXIT_FAILURE);
        }
        infoChilds[0].fd_from_parent[1] = -1;

        ProxyParent(infoChilds, nChilds);
    }

    free(infoChilds);
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
        fd_reader = open (path_input, O_RDONLY);
        if (fd_reader < 0) {
            perror("Error open");
            exit(EXIT_FAILURE);
        }
    }
    else
        fd_reader = infoChild->fd_from_parent[0];

    //TODO Dump
//    infoChild->fd_from_parent[0] = fd_reader;
//    printf("Child [%zu] : ", infoChild->numChild);
//    DumpFd(infoChild);

    //fcntl
    errno = 0;
    ret = fcntl(fd_reader, F_SETFL, O_RDONLY);
    if (ret < 0) {
        perror("Error fcntl");
        exit(EXIT_FAILURE);
    }

    errno = 0;
    ret = fcntl(fd_writer, F_SETFL, O_WRONLY);
    if (ret < 0) {
        perror("Error fcntl");
        exit(EXIT_FAILURE);
    }

    //splice
    ssize_t ret_splice = -1;
    while (ret_splice) {
        errno = 0;

        //TODO dump
        struct pollfd fdpoll;
        fdpoll.fd = fd_reader;
        fdpoll.events = POLLIN;
        poll(&fdpoll, 1, 0);
        if (infoChild->numChild == 1) {}
            //printf("Child revenst[%zu] 0x%x\n", infoChild->numChild, fdpoll.revents);
        //DumpFd(infoChild);

        ret_splice = splice(fd_reader, NULL, fd_writer, NULL, PIPE_BUF, SPLICE_F_MOVE);
        if (ret_splice < 0) {
            perror("Error splice");
            exit(EXIT_FAILURE);
        }

     //   printf("Child[%zu] ret_splice - %zu\n", infoChild->numChild, ret_splice);
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

  //  printf("Child[%zu] died\n", infoChild->numChild);
}


void ProxyParent(struct InfoChild *infoChilds, size_t nChilds)
{
    assert(infoChilds);

    //TODO Dump
//    for (size_t i = 0; i < nChilds; i++) {
//        printf("Parent [%zu] : ", i);
//        DumpFd(&infoChilds[i]);
//    }

    size_t nLinks = nChilds;

    struct InfoLink* IL = (struct InfoLink*)calloc(nLinks, sizeof(*IL));

    //Init IL[i_link]
    for (size_t i_link = 0; i_link < nLinks; i_link++) {

        IL[i_link].fd_writer = infoChilds[i_link].fd_to_parent[0];

        if (i_link == nLinks - 1)
            IL[i_link].fd_reader = STDOUT_FILENO;
        else
            IL[i_link].fd_reader = infoChilds[i_link + 1].fd_from_parent[1];

        size_t size_buffer = CountSizeBuffer(BASE_SIZE_BUFFER, i_link, nChilds);
        IL[i_link].buffer = (char*)calloc(size_buffer, sizeof(*IL[i_link].buffer));
        if (IL[i_link].buffer == NULL) {
            perror("Error calloc");
            exit(EXIT_FAILURE);
        }

        IL[i_link].buffer_end = IL[i_link].buffer + size_buffer;

        IL[i_link].cur_write = IL[i_link].buffer;
        IL[i_link].cur_read  = IL[i_link].buffer;

        IL[i_link].size_empty = size_buffer;
        IL[i_link].size_full  = 0;

        IL[i_link].isWasEOF = false;
    }

    size_t i_alive_child = 0;
    while (i_alive_child < nChilds) {
        //prepare poll
        size_t nWriters = nLinks;
        size_t nReaders = nLinks;

        struct pollfd* fdpoll_writers =
                (struct pollfd*)calloc(nWriters, sizeof(*fdpoll_writers));
        struct pollfd* fdpoll_readers =
                (struct pollfd*)calloc(nReaders, sizeof(*fdpoll_readers));

        for (size_t i_link = i_alive_child; i_link < nLinks; i_link++) {

            fdpoll_writers[i_link].fd = IL[i_link].fd_writer;
            fdpoll_writers[i_link].events = POLLIN;

            fdpoll_readers[i_link].fd     = IL[i_link].fd_reader;
            fdpoll_readers[i_link].events = POLLOUT;
        }

        //poll
        errno = 0;
        int ret_pool_writers = poll(fdpoll_writers, nWriters, 0);
        if (ret_pool_writers < 0) {
            perror("Error pool");
            exit(EXIT_FAILURE);
        }

        errno = 0;
        int ret_pool_readers = poll(fdpoll_readers, nReaders, 0);
        if (ret_pool_readers < 0) {
            perror("Error pool");
            exit(EXIT_FAILURE);
        }

        //read & write
        for (size_t i_link = i_alive_child; i_link < nLinks; i_link++) {

            //Closed Child Writer
            bool isPipeClosed_ChildWriter = fdpoll_writers[i_link].revents == POLLHUP;
            if (isPipeClosed_ChildWriter) {
                errno = 0;
                int ret = close(IL[i_link].fd_writer);
                if (ret < 0) {
                    perror("Error close");
                    exit(EXIT_FAILURE);
                }
                IL[i_link].fd_writer = -1;
            }

            //write to buffer
            bool isCanWrite = fdpoll_writers[i_link].revents & POLLIN;
            if (isCanWrite && IL[i_link].size_empty > 0)
                WriteToBuffer(&IL[i_link]);

            // read from buffer
            bool isCanRead = fdpoll_readers[i_link].revents ==
                             fdpoll_readers[i_link].events;

            if (isCanRead && IL[i_link].size_full > 0)
                ReadFromBuffer(&IL[i_link]);

            //Close Pipe to ChildReader
            if (IL[i_link].fd_writer == -1 && IL[i_link].size_full == 0) {

                //printf("End [%zu]\n", i_link);

//                if (!IL[i_link].isWasEOF) {
//                    perror("Data loss");
//                    exit(EXIT_FAILURE);
//                }

                if (i_link != i_alive_child) {
                    perror("Child died");
                    exit(EXIT_FAILURE);
                }

                errno = 0;
                int ret = close(IL[i_link].fd_reader);
                if (ret < 0) {
                    perror("Error close");
                    exit(EXIT_FAILURE);
                }
                IL[i_link].fd_reader = -1;

                i_alive_child++;
            }
        }

        free(fdpoll_writers);
        free(fdpoll_readers);
    }
}


void  WriteToBuffer(struct InfoLink* IL)
{
    assert(IL);

    errno = 0;
    ssize_t ret_read = read(IL->fd_writer, IL->cur_write, IL->size_empty);
    if (ret_read < 0) {
        perror("Error read");
        exit(EXIT_FAILURE);
    }

    //TODO dump
//    FILE* fd_debug = fopen("debug.txt", "a");
//    fprintf(fd_debug, "[%d] ", *(IL->cur_write + ret_read));
//    fclose(fd_debug);

//    if (*(IL->cur_write + ret_read) == EOF)
//        IL->isWasEOF = true;

    if (IL->cur_write >= IL->cur_read)
        IL->size_full += ret_read;

    if (IL->cur_write + ret_read == IL->buffer_end) {
        IL->cur_write  = IL->buffer;
        IL->size_empty = IL->cur_read - IL->cur_write;
    }
    else {
        IL->cur_write  += ret_read;
        IL->size_empty -= ret_read;
    }
    int a = 0;
}



void ReadFromBuffer(struct InfoLink* IL)
{
    assert(IL);

    errno = 0;
    ssize_t ret_write = write(IL->fd_reader, IL->cur_read, IL->size_full);
    if (ret_write < 0) {
        perror("Error write");
        exit(EXIT_FAILURE);
    }

    if (IL->cur_read >= IL->cur_write)
        IL->size_empty += ret_write;

    if (IL->cur_read + ret_write == IL->buffer_end) {
        IL->cur_read  = IL->buffer;
        IL->size_full = IL->cur_write - IL->cur_read;
    }
    else {
        IL->cur_read  += ret_write;
        IL->size_full -= ret_write;
    }
}



size_t CountSizeBuffer(size_t base_size, size_t iChild, size_t nChild)
{
    size_t size = pow(3, (double)(nChild - iChild)) * 1024;
    return size;
}



// Work with pipe {

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
    infoChild->fd_from_parent[0] = -1;

    errno = 0;
    ret = close(infoChild->fd_to_parent[1]);
    if (ret < 0) {
        perror("Error close");
        exit(EXIT_FAILURE);
    }
    infoChild->fd_to_parent[1] = -1;
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
    infoChild->fd_from_parent[1] = -1;

    errno = 0;
    ret = close(infoChild->fd_to_parent[0]);
    if (ret < 0) {
        perror("Error close");
        exit(EXIT_FAILURE);
    }
    infoChild->fd_to_parent[0] = -1;
}

// } Work with pipe



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

    sprintf(strDump, "\n");

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
