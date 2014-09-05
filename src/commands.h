#ifndef PICOFTPD_COMMANDS_H_INCLUDED
#define PICOFTPD_COMMANDS_H_INCLUDED

#define _GNU_SOURCE

#include "state.h"

typedef int (*ftp_command_func_t)(const char *, ftp_state_t *);

typedef struct {
    const char* name;
    ftp_command_func_t func;
} ftp_command_t;

extern ftp_command_t picoftpd_commands[];

typedef enum{
    FTP_STATUS_OK = 200,
    FTP_STATUS_CLOSING = 221, 
    FTP_STATUS_ENTERING_PASSIVE = 227, 
    FTP_STATUS_USER_LOGGED_IN = 230,
    FTP_STATUS_INVALID_SYNTAX = 500,
    FTP_STATUS_INVALID_SYNTAX_PARAMS = 501,
    FTP_STATUS_COMMAND_NOT_IMPLEMENTED = 502,
    FTP_STATUS_INVALID_COMMAND_SEQUENCE = 503,
    FTP_STATUS_PARAMETER_NOT_IMPLEMENTED = 504,
    FTP_COMMAND_QUIT = 0,
    FTP_COMMAND_EOF = -1,
    FTP_COMMAND_OOM = -2,
} ftp_status_t;

#endif
