#ifndef PICOFTPD_UTIL_H_INCLUDED
#define PICOFTPD_UTIL_H_INCLUDED

#if defined(__GNUC__)
    #define UNUSED __attribute__((unused))
#else
    #define UNUSED
#endif

#include "state.h"

ssize_t str_remove_suffix(char *str, const char *suffix);

void ftp_debug(ftp_state_t *state, const char *format, ...);
void ftp_debug_perror(ftp_state_t *state, const char *msg);

#endif
