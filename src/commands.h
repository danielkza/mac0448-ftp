#ifndef PICOFTPD_COMMANDS_H_INCLUDED
#define PICOFTPD_COMMANDS_H_INCLUDED

#define _GNU_SOURCE
#include "state.h"

typedef enum {
    FTP_RESULT_OK = 0,
    FTP_RESULT_EOF,
    FTP_RESULT_ABORT,
    FTP_RESULT_ABORT_TRANSFER
} ftp_result_t;

typedef ftp_result_t (*ftp_command_func_t)(const char *arg, ftp_state_t *state);

typedef struct {
    const char* name;
    ftp_command_func_t func;
    bool needs_arg;
} ftp_command_t;

extern const ftp_command_t picoftpd_commands[];

typedef enum{
    FTP_STATUS_TRANSFER_STARTING = 125,
    FTP_STATUS_OK = 200,
    FTP_STATUS_SYSTEM_NAME = 215,
    FTP_STATUS_CLOSING = 221,
    FTP_STATUS_TRANSFER_SUCCEEDED = 226,
    FTP_STATUS_ENTERING_PASSIVE = 227,
    FTP_STATUS_USER_LOGGED_IN = 230,
    FTP_STATUS_SERVICE_NOT_AVAILABLE = 421,
    FTP_STATUS_CANT_OPEN_DATA_CONNECTION = 425,
    FTP_STATUS_TRANSFER_FAILED = 426,
    FTP_STATUS_LOCAL_ERROR = 451,
    FTP_STATUS_INVALID_SYNTAX = 500,
    FTP_STATUS_INVALID_SYNTAX_PARAMS = 501,
    FTP_STATUS_COMMAND_NOT_IMPLEMENTED = 502,
    FTP_STATUS_INVALID_COMMAND_SEQUENCE = 503,
    FTP_STATUS_PARAMETER_NOT_IMPLEMENTED = 504,
    FTP_STATUS_PERMANENT_FILE_UNAVAILABLE = 550,
} ftp_status_t;

ftp_result_t ftp_write_response(ftp_state_t *state, ftp_status_t code,
                                const char *format, ...);

ftp_result_t ftp_hello(ftp_state_t *state);
ftp_result_t ftp_quit(const char *arg, ftp_state_t *state);
#endif
