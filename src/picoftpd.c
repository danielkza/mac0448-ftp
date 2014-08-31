#include "config.h"

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    printf("Hello, World!\n");

    for(int i = 1; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }
    printf("\n");

    return EXIT_SUCCESS;
}
