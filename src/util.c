#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"

ssize_t str_remove_suffix(char *str, const char *suffix)
{
    assert(str != NULL);
    assert(suffix != NULL);

    char *str_end = strchr(str, '\0'),
    *suffix_end = strchr(suffix, '\0');

    while((str_end != str || suffix_end != suffix)
          && *(str_end - 1) == *(suffix_end - 1)) {
        --str_end;
        --suffix_end;
    }

    if(suffix_end == suffix) {
        *str_end = '\0';
        return str_end - str;
    } else {
        return -1;
    }
}

void ftp_debug(ftp_state_t *state, const char *format, ...)
{
    assert(state != NULL);
    assert(format != NULL);

    fprintf(stderr, "%d: ", state->control_socket);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

void ftp_debug_perror(ftp_state_t *state, const char *msg)
{
    assert(state != NULL);

    int errno_saved = errno;

    fprintf(stderr, "%d: ", state->control_socket);

    errno = errno_saved;
    perror(msg);
    errno = errno_saved;
}
