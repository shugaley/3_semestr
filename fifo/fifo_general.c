
#include "fifo_general.h"

static const char   PATH_BASE_FIFO[] = "fifo_"; //fifo_<pid>
static const size_t SIZE_STR_INT32   = 7;
static const time_t TIME_WAITING     = 1;

//=============================================================================
//General func {

char* MakePathFifo(const pid_t pid)
{
    size_t size_pathFifo = sizeof(PATH_BASE_FIFO) + SIZE_STR_INT32 + 1;
    char* pathFifo = (char*)calloc(size_pathFifo, sizeof(*pathFifo));

    strcpy  (pathFifo, PATH_BASE_FIFO);
    snprintf(pathFifo + sizeof(PATH_BASE_FIFO) - 1,
             size_pathFifo - strlen(pathFifo),
             "%d", pid);

    return pathFifo;
}

bool IsCanReadFile(int fd_read)
{
    fd_set readfds;

    int n = fd_read + 1;
    FD_ZERO(         &readfds);
    FD_SET (fd_read, &readfds);

    struct timeval tv_Fifo;
    tv_Fifo.tv_sec  = TIME_WAITING;
    tv_Fifo.tv_usec = 0;

    int ret_select = select(n, &readfds, NULL, NULL, &tv_Fifo);
    if(ret_select <= 0)
        return false;
    else
        return true;
}

size_t CountSizeFile(const char* path)
{
    assert(path);

    struct stat stat_File = {};

    int ret_stat = stat(path, &stat_File);
    if (ret_stat == -1) {
        const char strErrorBase[] = "Error count size file ";
        char strError[PATH_MAX + sizeof(strErrorBase)] = "";
        strcat(strError, strErrorBase);
        strcat(strError, path);
        perror(strError);
        exit(EXIT_FAILURE);
    }

    size_t sizeFile = stat_File.st_size;
    return sizeFile;
}

//Shell funcs {

int Mkfifo(const char* path, mode_t mode, const char* strError)
{
    errno = 0;
    int ret_mkfifo = mkfifo(path, mode);
    if (ret_mkfifo != 0 && errno != EEXIST) {
        perror(strError);
        exit(EXIT_FAILURE);
    }

    return ret_mkfifo;
}


int Open(const char* path, int flag, const char* strError)
{
    (void) umask(0);
    int fd = open(path, flag);
    if(fd < 0) {
        perror(strError);
        exit(EXIT_FAILURE);
    }

    return fd;
}


ssize_t Read(int fd, void *buf, size_t nbytes, const char* strError)
{
    ssize_t ret_read = read(fd, buf, nbytes);
    if(ret_read <= 0) {
        perror(strError);
        exit(EXIT_FAILURE);
    }

    return ret_read;
}


ssize_t Write(int fd, const void *buf, size_t n, const char* strError)
{
    ssize_t ret_write = write(fd, buf, n);
    if(ret_write <= 0) {
        perror(strError);
        exit(EXIT_FAILURE);
    }

    return ret_write;
}


int Fcntl(int fd, int cmd, long arg, const char* strError)
{
    int ret_fcntl = fcntl(fd, cmd, arg);
    if(ret_fcntl == -1) {
        perror(strError);
        exit(EXIT_FAILURE);
    }

    return ret_fcntl;
}
// } Shell   funcs
// } General funcs


