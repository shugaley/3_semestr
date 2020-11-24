
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
#include <sys/wait.h>
#include <unistd.h>

#include "proxy.h"

const int BASE_SIZE_BUFFER = 1024;

struct InfoLink {
    int    fd_reader;
    int    fd_writer;

    char*  buffer;
    char*  buffer_end;

    char*  cur_write; //from buffer
    char*  cur_read;  //to buffer

    size_t size_empty;
    size_t size_full;
};


struct InfoChild {
    size_t numChild;
    pid_t  pid_child;

    int    fd_to_parent[2];
    int    fd_from_parent[2];
};

void ProxyChild (const char* path_input, struct InfoChild* infoChild);
void ProxyParent(const struct InfoChild* infoChilds, size_t nChilds);

void ReadToBuffer   (struct InfoLink* IL);
void WriteFromBuffer(struct InfoLink* IL);

size_t CountSizeBuffer(size_t base_size, size_t iChild, size_t nChild);
void AnalizeStatusChild(int status, const struct InfoChild* infoChild);

// Work with fd pipe {
void MakeConnectionPipes         (struct InfoChild* infoChild);
void CloseRedundantFdPipes_Parent(struct InfoChild* infoChild);
void CloseRedundantFdPipes_Child (struct InfoChild* infoChild);
// } Work with fd pipe

// Shell funcs {
void DetectDeathParent(pid_t pid_parent, int signal);
// } Shell funcs

void DumpFd(struct InfoChild* infoChild);

//-----------------------------------------------------------------------------

void ProxyChilds(const char* path_input, size_t nChilds)
{
    struct InfoChild* infoChilds =
            (struct InfoChild*)calloc(nChilds * 2, sizeof(*infoChilds));

    struct InfoChild infoChildCur = {};
    for (size_t iChild = 0; iChild < nChilds; iChild++) {

        infoChildCur.numChild  = iChild;
        MakeConnectionPipes(&infoChildCur);

        pid_t pid_parent = getpid();
        pid_t ret_fork = 0;

        switch(ret_fork = fork()) {
        case -1:
            perror("Error fork");
            exit(EXIT_FAILURE);

        case 0:
            DetectDeathParent(pid_parent, SIGTERM);

            for (size_t jChild = 0; jChild < infoChildCur.numChild; jChild++)
                CloseRedundantFdPipes_Child(&infoChilds[jChild]);
            free(infoChilds);
            CloseRedundantFdPipes_Child(&infoChildCur);

            ProxyChild(path_input, &infoChildCur);
            exit(EXIT_SUCCESS);

        default:
            infoChildCur.pid_child = ret_fork;
            CloseRedundantFdPipes_Parent(&infoChildCur);
            infoChilds[iChild] = infoChildCur;
        }
    }

    errno = 0;
    int ret = close(infoChilds[0].fd_from_parent[1]);
    if (ret < 0) {
        perror("Error close");
        exit(EXIT_FAILURE);
    }
    infoChilds[0].fd_from_parent[1] = -1;

    ProxyParent(infoChilds, nChilds);

    free(infoChilds);
}

//-----------------------------------------------------------------------------

void ProxyChild(const char* path_input, struct InfoChild* infoChild)
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
        ret_splice = splice(fd_reader, NULL, fd_writer, NULL,
                            PIPE_BUF, SPLICE_F_MOVE);
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


void ProxyParent(const struct InfoChild* infoChilds, size_t nChilds)
{
    assert(infoChilds);

    size_t nLinks = nChilds;
    struct InfoLink* IL = (struct InfoLink*)calloc(nLinks, sizeof(*IL));

    //Init IL[i_link]
    for (size_t i_link = 0; i_link < nLinks; i_link++) {

        IL[i_link].fd_reader = infoChilds[i_link].fd_to_parent[0];

        if (i_link == nLinks - 1)
            IL[i_link].fd_writer = STDOUT_FILENO;
        else
            IL[i_link].fd_writer = infoChilds[i_link + 1].fd_from_parent[1];

        size_t size_buffer = CountSizeBuffer(BASE_SIZE_BUFFER, i_link, nChilds);
        IL[i_link].buffer =
                (char*)calloc(size_buffer, sizeof(*IL[i_link].buffer));
        if (IL[i_link].buffer == NULL) {
            perror("Error calloc");
            exit(EXIT_FAILURE);
        }

        IL[i_link].buffer_end = IL[i_link].buffer + size_buffer;

        IL[i_link].cur_read  = IL[i_link].buffer;
        IL[i_link].cur_write = IL[i_link].buffer;

        IL[i_link].size_empty = size_buffer;
        IL[i_link].size_full  = 0;
    }


    size_t i_alive_child = 0;
    while (i_alive_child < nChilds) {
        //prepare select
        fd_set fd_readers, fd_writers;
        FD_ZERO(&fd_readers);
        FD_ZERO(&fd_writers);

        size_t maxfd = 0;
        for (size_t i_link = i_alive_child; i_link < nLinks; i_link++) {

            if (IL[i_link].fd_reader != -1 && IL[i_link].size_empty > 0) {
                FD_SET(IL[i_link].fd_reader, &fd_readers);
                if(IL[i_link].fd_reader > maxfd)
                    maxfd = IL[i_link].fd_reader;
            }

            if (IL[i_link].fd_writer != -1 && IL[i_link].size_full > 0) {
                FD_SET(IL[i_link].fd_writer, &fd_writers);
                if(IL[i_link].fd_writer > maxfd)
                    maxfd = IL[i_link].fd_writer;
            }
        }

        //select
        errno = 0;
        int ret_select = select(maxfd + 1, &fd_readers, &fd_writers, NULL, NULL);
        if (ret_select < 0) {
            perror("Error select");
            exit(EXIT_FAILURE);
        }

        //read & write
        for (size_t i_link = i_alive_child; i_link < nLinks; i_link++) {

            //Read to buffer
            bool isCanRead = FD_ISSET(IL[i_link].fd_reader, &fd_readers);

            if (isCanRead && IL[i_link].size_empty > 0)
                ReadToBuffer(&IL[i_link]);

            //Write from buffer
            bool isCanWrite = FD_ISSET(IL[i_link].fd_writer, &fd_writers);

            if (isCanWrite && IL[i_link].size_full > 0)
                WriteFromBuffer(&IL[i_link]);

            //Close Pipe to ChildReader
            if (IL[i_link].fd_reader == -1 && IL[i_link].size_full == 0) {

                if (i_link != i_alive_child) {
                    perror("Child died");
                    exit(EXIT_FAILURE);
                }

                errno = 0;
                int ret = close(IL[i_link].fd_writer);
                if (ret < 0) {
                    perror("Error close");
                    exit(EXIT_FAILURE);
                }
                IL[i_link].fd_writer = -1;

                i_alive_child++;
            }
        }
    }

    for (size_t i_link = 0; i_link < nLinks; i_link++)
        free(IL[i_link].buffer);
    free(IL);

    for (size_t i_child = 0; i_child < nChilds; i_child++) {
        errno = 0;
        int status_child = 0;
        int ret = waitpid(infoChilds[i_child].pid_child, &status_child, 0);
        if (ret < 0) {
            perror("Error waitpid");
            exit(EXIT_FAILURE);
        }

        AnalizeStatusChild(status_child, &infoChilds[i_child]);
    }
}


void ReadToBuffer(struct InfoLink* IL)
{
    assert(IL);

    errno = 0;
    ssize_t ret_read = read(IL->fd_reader, IL->cur_read, IL->size_empty);
    if (ret_read < 0) {
        perror("Error read");
        exit(EXIT_FAILURE);
    }

    //If pipe closed by ChildWriter
    if (ret_read == 0) {
        close(IL->fd_reader);
        IL->fd_reader = -1;
        return;
    }

    if (IL->cur_read >= IL->cur_write)
        IL->size_full += ret_read;

    if (IL->cur_read + ret_read == IL->buffer_end) {
        IL->cur_read  = IL->buffer;
        IL->size_empty = IL->cur_write - IL->cur_read;
    }
    else {
        IL->cur_read  += ret_read;
        IL->size_empty -= ret_read;
    }
}



void WriteFromBuffer(struct InfoLink* IL)
{
    assert(IL);

    errno = 0;
    ssize_t ret_write = write(IL->fd_writer, IL->cur_write, IL->size_full);
    if (ret_write < 0 && errno != EAGAIN) {
        perror("Error write");
        exit(EXIT_FAILURE);
    }

    if (IL->cur_write >= IL->cur_read)
        IL->size_empty += ret_write;

    if (IL->cur_write + ret_write == IL->buffer_end) {
        IL->cur_write  = IL->buffer;
        IL->size_full = IL->cur_read - IL->cur_write;
    }
    else {
        IL->cur_write  += ret_write;
        IL->size_full -= ret_write;
    }
}

void AnalizeStatusChild(int status, const struct InfoChild* infoChild)
{
    assert(infoChild);

    if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
        return;

    fprintf(stderr, "Warning maybe loss data\n");
    fprintf(stderr, "Child with number [%zu] ",        infoChild->numChild);
    fprintf(stderr, "with pid [%d] exit not success ", infoChild->pid_child);
    fprintf(stderr, "with exit status == %d\n", WEXITSTATUS(status));

    if (WIFSIGNALED(status))
        fprintf(stderr, "Killed by signal %d\n", WTERMSIG(status));
}


size_t CountSizeBuffer(size_t base_size, size_t iChild, size_t nChild)
{
    size_t size = pow(3, (double)(nChild - iChild)) * 1024;
    return size;
}


// Work with fd pipe {

void MakeConnectionPipes(struct InfoChild* infoChild)
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


void CloseRedundantFdPipes_Parent(struct InfoChild* infoChild)
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


void CloseRedundantFdPipes_Child(struct InfoChild* infoChild)
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

// } Work with fd pipe



// Shell funcs {

void DetectDeathParent(pid_t pid_parent, int signal)
{
    int ret = prctl(PR_SET_PDEATHSIG, signal);
    if (ret < 0) {
        perror("Error ret()");
        exit(EXIT_FAILURE);
    }
    if (pid_parent != getppid()) {
        perror("Error parent process");
        exit(EXIT_FAILURE);
    }
}

// } Shell funcs



void DumpFd(struct InfoChild* infoChild)
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
