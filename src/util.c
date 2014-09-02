#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

static const char out_of_memory_msg[] = "Out of memory, aborting!\n";

char *strdup_e(const char *in)
{
    char *out = strdup(in);
    if(out == NULL) {
        fputs(out_of_memory_msg, stderr);
        abort();
    }

    return out;
}

void *malloc_e(size_t size)
{
    void *m = malloc(size);
    if(m == NULL) {
        fputs(out_of_memory_msg, stderr);
        abort();
    }

    return m;
}
