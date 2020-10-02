#pragma once
#ifndef FIFO_FIFO_GENERAL_H
#define FIFO_FIFO_GENERAL_H

#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


//General funcs {

char* MakePathFifo (const pid_t pid);
bool  IsCanReadFile(int fd_read);
size_t CountSizeFile(const char* path);

//Shell funcs {

int Mkfifo(const char* path, mode_t mode, const char* strError);
int Open  (const char* path, int flag,      const char* strError);

ssize_t Read (int fd,       void* buf, size_t nbytes, const char* strError);
ssize_t Write(int fd, const void* buf, size_t n,      const char* strError);

int Fcntl(int fd, int cmd, long arg, const char* strError);


// } Shell funcs
// } General funcs

#endif //FIFO_FIFO_GENERAL_H
