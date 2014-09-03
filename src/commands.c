#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <strings.h>

#include "commands.h"
#include "state.h"
#include "util.h"

int ftp_noop(UNUSED const char *arg, UNUSED ftp_state_t *state)
{
    return FTP_STATUS_OK;
}

int ftp_quit(UNUSED const char *arg, UNUSED ftp_state_t *state)
{
    return FTP_STATUS_CLOSING;
}

int ftp_not_implemented(UNUSED const char *arg, UNUSED ftp_state_t *state)
{
    return FTP_STATUS_COMMAND_NOT_IMPLEMENTED;
}

int ftp_user(const char *arg, ftp_state_t *state)
{
    if(arg == NULL)
        return FTP_STATUS_INVALID_SYNTAX_PARAMS;
    
    const char *user = arg;
    if(strlen(user) == 0)
        user = NULL;

    printf("Setting user to: %s\n", user != NULL ? user : "NULL");
    ftp_state_set_username(state, user);
    ftp_state_set_password(state, NULL);

    return FTP_STATUS_USER_LOGGED_IN;
}

int ftp_pass(const char *arg, ftp_state_t *state)
{
    if(arg == NULL)
        return FTP_STATUS_INVALID_SYNTAX_PARAMS;
    
    const char *pass = arg;
    if(strlen(pass) == 0)
        pass = NULL;

    if(state->username == NULL)
        return FTP_STATUS_INVALID_COMMAND_SEQUENCE;

    ftp_state_set_password(state, pass);
    return FTP_STATUS_USER_LOGGED_IN;
}

int ftp_type(const char *arg, ftp_state_t *state)
{
    if(arg == NULL)
       return FTP_STATUS_INVALID_SYNTAX_PARAMS;
    
    if(strcasecmp(arg, "i") == 0) {
       ftp_state_set_type_image(state, 1);
       return FTP_STATUS_OK;
    } else if (strncasecmp(arg, "a ", 2) == 0 || strncasecmp(arg, "e ", 2) == 0 || strncasecmp(arg, "l ", 2) == 0) {
       return FTP_STATUS_PARAMETER_NOT_IMPLEMENTED;
    } else { 
       return FTP_STATUS_INVALID_SYNTAX_PARAMS;
    }
}

int ftp_mode(const char *arg, ftp_state_t *state)
{
    if(arg == NULL)
       return FTP_STATUS_INVALID_SYNTAX_PARAMS;
    
    if(strcasecmp(arg, "s") == 0) {
       ftp_state_set_mode_stream(state, 1);
       return FTP_STATUS_OK;
    } else if (strcasecmp(arg, "b") == 0 || strcasecmp(arg, "c") == 0) {
       return FTP_STATUS_PARAMETER_NOT_IMPLEMENTED;
    } else { 
       return FTP_STATUS_INVALID_SYNTAX_PARAMS;
    }
}

int ftp_stru(const char *arg, ftp_state_t *state)
{
    if(arg == NULL)
       return FTP_STATUS_INVALID_SYNTAX_PARAMS;
    
    if(strcasecmp(arg, "f") == 0) {
       ftp_state_set_structure_file(state, 1);
       return FTP_STATUS_OK;
    } else if (strcasecmp(arg, "r") == 0 || strcasecmp(arg, "p") == 0) {
       return FTP_STATUS_PARAMETER_NOT_IMPLEMENTED;
    } else {
       return FTP_STATUS_INVALID_SYNTAX_PARAMS;
    }
}

ftp_command_t picoftpd_commands[] = {
    {"NOOP", &ftp_noop},
    {"USER", &ftp_user},
    {"PASS", &ftp_pass},
    {"TYPE", &ftp_type},
    {"MODE", &ftp_mode},
    {"STRU", &ftp_stru},
    {"QUIT", &ftp_quit}, 
    {NULL, NULL}, 
};
