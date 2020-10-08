#pragma once
#ifndef MESSAGE_MESSAGE_H
#define MESSAGE_MESSAGE_H

#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "numbers.h"

void Print_NumChildProcesses(const size_t nProcesses);

#endif //MESSAGE_MESSAGE_H