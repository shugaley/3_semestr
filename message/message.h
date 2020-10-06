#pragma once
#ifndef MESSAGE_MESSAGE_H
#define MESSAGE_MESSAGE_H

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#include "numbers.h"

void Print_NumChildProcesses(const size_t nProcesses);

#endif //MESSAGE_MESSAGE_H