
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "proxy.h"


struct Connect {
    size_t size_buffer;
    int fr_writer;
    int fd_reader;

};

struct InfoChild {
    size_t numChild;
    pid_t  pid_child;
    int fd_writers[2];
    int fd_readers[2];
};


struct InfoChild* MakeInfoChilds(size_t nChilds);

size_t CountSizeBuffer(size_t maxsize, size_t iChild, size_t nChild);


void ProxyChilds(const char* path_input, size_t nChilds)
{
    assert(path_input);

    struct InfoChild* infoChilds = (struct InfoChild*)calloc(nChilds * 2,
                                                             sizeof(*infoChilds));

    for (size_t iChild = 0; iChild < nChilds; iChild++) {
        int ret = 0;

//        //create pipe
//        errno = 0;
//        ret = pipe(infoChilds[iChild].fdpipe_input);
//        if (ret < 0) {
//            perror("Error pipe");
//            exit(EXIT_FAILURE);
//        }
//        errno = 0;
//        ret = pipe(infoChilds[iChild].fdpipe_ouput);
//        if (ret < 0) {
//            perror("Error pipe");
//            exit(EXIT_FAILURE);
//        }

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

        default:
            ;
        }
    }



}

size_t CountSizeBuffer(size_t maxsize, size_t iChild, size_t nChild)
{
    size_t size = pow(3, (double)(nChild - iChild)) * 1024;
    return size;
}


