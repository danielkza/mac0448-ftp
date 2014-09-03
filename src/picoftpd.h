#ifndef _PICOFTPD_H
#define _PICOFTPD_H

#define _GNU_SOURCE

#include "state.h"

typedef int (*ftp_command_func_t)(const char *, ftp_state_t *);

typedef struct {
    const char name[5];
    ftp_command_func_t func;
} ftp_command_t;

#endif
