#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>

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

int ftp_pasv(UNUSED const char *arg, ftp_state_t *state)
{
    if(state->data_listen_socket != -1) {
        close(state->data_listen_socket);
        state->data_listen_socket = -1;
    }
    
    return ftp_state_open_data_socket(state);
}

void ftp_start_transfer(ftp_state_t *state, FILE *read_file, FILE *write_file)
{
    if((state->data_pid = fork()) == -1) {
        perror("fork: ");
        abort();
    }
    
    if(state->data_pid > 0)
        return;
    
    int ret = 1;
    
    int data_socket;
    if((data_socket = accept(state->data_listen_socket, (struct sockaddr *) NULL, NULL)) == -1)
        goto cleanup;
    
    char c;
    ssize_t len;

    if(read_file != NULL) {
        for(;;) {
            int ic = fgetc(read_file);
            if(ic == EOF) {
                if(!ferror(read_file))
                    ret = 0;
                
                break;
            }
            
            c = (char)ic;
            len = write(data_socket, &c, 1);
            if(len == -1) {
                break;
            }   
        }
    } else if(write_file != NULL) {
        for(;;) {
            len = read(data_socket, &c, 1);
            fputc(c, stdout);
            if(len > 0) {
                if(fputc(c, write_file) == EOF) {
                    break;
                }
            } else if(len == 0) {
                ret = 0;
                break;
            } else {
                break;
            }
        }
    }
    
    close(data_socket);

cleanup:
    if(read_file!= NULL)
        fclose(read_file);

    if(write_file != NULL)
        fclose(write_file);
    
    exit(ret);
}

int ftp_retr(const char *arg, ftp_state_t *state)
{       
    if(arg == NULL || arg[0] == '\0')
        return FTP_STATUS_INVALID_SYNTAX_PARAMS;
    
    if(!state->is_type_image || !state->is_mode_stream
       || !state->is_structure_file || state->data_listen_socket == -1)
        return FTP_STATUS_INVALID_COMMAND_SEQUENCE;
        
    if(state->data_pid != 0)
        return FTP_STATUS_INVALID_COMMAND_SEQUENCE;
    
    FILE *file = fopen(arg, "rb");
    if(file == NULL) {
        printf("file unavailable\n");
        return FTP_STATUS_PERMANENT_FILE_UNAVAILABLE;
    }
    
    ftp_start_transfer(state, file, NULL);
    fclose(file);
    
    return FTP_STATUS_TRANSFER_STARTING;
}

int ftp_stor(const char *arg, ftp_state_t *state)
{       
    if(arg == NULL || arg[0] == '\0')
        return FTP_STATUS_INVALID_SYNTAX_PARAMS;
    
    if(!state->is_type_image || !state->is_mode_stream
       || !state->is_structure_file || state->data_listen_socket == -1)
        return FTP_STATUS_INVALID_COMMAND_SEQUENCE;
        
    if(state->data_pid != 0)
        return FTP_STATUS_INVALID_COMMAND_SEQUENCE;
    
    FILE *file = fopen(arg, "wb");
    if(file == NULL)
        return FTP_STATUS_PERMANENT_FILE_UNAVAILABLE;
    
    ftp_start_transfer(state, NULL, file);
    fclose(file);
    
    return FTP_STATUS_TRANSFER_STARTING;
}

ftp_command_t picoftpd_commands[] = {
    {"NOOP", &ftp_noop},
    {"USER", &ftp_user},
    {"PASS", &ftp_pass},
    {"TYPE", &ftp_type},
    {"MODE", &ftp_mode},
    {"STRU", &ftp_stru},
    {"QUIT", &ftp_quit},
    {"PASV", &ftp_pasv},
    {"RETR", &ftp_retr},
    {"STOR", &ftp_stor},
    {NULL, NULL}, 
};
