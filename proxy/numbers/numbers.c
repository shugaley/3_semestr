
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "numbers.h"

size_t Strtoul(const char* strNumber)
{
    assert(strNumber);

    char* endptr_keyNumber = (char*)calloc(1, sizeof(char));
    size_t nNumber = strtoul(strNumber, &endptr_keyNumber, 10);

    if(errno != 0) {
        perror("ERROR! Wrong key (big)\n");
        exit(EXIT_FAILURE);
    }

    if(*endptr_keyNumber != '\0') {
        perror("ERROR! Wrong key (not number)\n");
        exit(EXIT_FAILURE);
    }

    if(nNumber <= 0) {
        perror("ERROR! Wrong key (number below 0)\n");
        exit(EXIT_FAILURE);
    }

    return nNumber;
}


void PrintNumbers(size_t nNumbers)
{
    for(size_t i_Number = 0; i_Number < nNumbers; i_Number++)
        fprintf(stdout, "%zu ", i_Number + 1);
    fprintf(stdout, "\n");
}