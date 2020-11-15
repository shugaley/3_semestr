
#include <stdio.h>
#include <stdlib.h>

#include "numbers.h"

#include "proxy.h"

int main(int argc, char** argv)
{
    if (argc != 3) {
        perror("Wrong number of keys");
        exit(EXIT_FAILURE);
    }

    size_t nChilds = Strtoul(argv[1]);
    char* path_input = argv[2];

    ProxyChilds(path_input, nChilds);

    return 0;
}
