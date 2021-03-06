#pragma once
#ifndef FIFO_FIFO_H
#define FIFO_FIFO_H

#include "fifo_general.h"

static const char PATH_FILE_FIFO_TRANSFER_PID[] = "/tmp/FifoTransferPid";
static const mode_t MKFIFO_MODE_DEFAULT = 0666;

void WriteFifo(const char* pathInput);
void ReadFifo();


#endif //FIFO_FIFO_GENERAL_H
