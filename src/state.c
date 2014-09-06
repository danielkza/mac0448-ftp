#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "util.h"
#include "state.h"
#include "commands.h"

ftp_state_t* ftp_state_new(void)
{
    ftp_state_t *state = malloc_e(sizeof(*state));

    state->username = NULL;
    state->password = NULL;
    state->curdir = NULL;
    
    state->is_mode_stream = 1;
    state->is_type_image = 0;
    state->is_structure_file = 1;
    
    state->data_listen_socket = -1;
    state->control_socket = -1;
    state->data_pid = 0;
    
    return state;
}

void ftp_state_set_username(ftp_state_t *state, const char *username)
{
    assert(state != NULL);

    if(state->username != NULL)
        free(state->username);

    if(username != NULL)
        state->username = strdup_e(username);
}

void ftp_state_set_password(ftp_state_t *state, const char *password)
{
    assert(state != NULL);

    if(state->password != NULL)
        free(state->password);

    if(password != NULL)
        state->password = strdup_e(password);
}

int ftp_state_set_curdir(ftp_state_t *state, const char *curdir)
{
    assert(state != NULL);
    assert(curdir != NULL);

    if(strlen(curdir) == 0)
        return 0;

    if(chdir(curdir) != 0)
        return 0;

    if(state->curdir != NULL)
        free(state->curdir);

    state->curdir = strdup_e(curdir);
    return 1;
}

int ftp_state_set_type_image(ftp_state_t *state, int is_image)
{
    assert(state != NULL);
    
    int old = state->is_type_image;
    state->is_type_image = (is_image != 0) ? 1 : 0;
    
    return old;
}

int ftp_state_set_mode_stream(ftp_state_t *state, int is_stream)
{
    assert(state != NULL);
    
    int old = state->is_mode_stream;
    state->is_mode_stream = (is_stream != 0) ? 1 : 0;
    
    return old;
}

int ftp_state_set_structure_file(ftp_state_t *state, int is_file)
{
    assert(state != NULL);
    
    int old = state->is_structure_file;
    state->is_structure_file = (is_file != 0) ? 1 : 0;
    
    return old;
}

int ftp_state_open_data_socket(ftp_state_t *state)
{
    printf("open_data_socket\n");
    
    if(state->data_listen_socket != -1 || state->data_pid != 0 || state->control_socket == -1)
        return FTP_STATUS_INVALID_COMMAND_SEQUENCE;
    
    struct sockaddr_in data_addr = {0};
    socklen_t data_addr_len = sizeof(data_addr);
    if(getsockname(state->control_socket, (struct sockaddr *)&data_addr, &data_addr_len) == -1) {
        printf("getsockname: ");
        return FTP_STATUS_CANT_OPEN_DATA_CONNECTION;
    }
    
    data_addr.sin_port = htons(0);
    
    int data_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(data_socket == -1) {
        printf("socket: ");
        return FTP_STATUS_CANT_OPEN_DATA_CONNECTION;
    }
    
    if(bind(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr)) == -1) {
        printf("bind: ");
        close(data_socket);
        
        return FTP_STATUS_CANT_OPEN_DATA_CONNECTION;
    }
       
    if(listen(data_socket, 1) == -1) {
        printf("listen: ");
        close(data_socket);
        
        return FTP_STATUS_CANT_OPEN_DATA_CONNECTION;
    }
     
    state->data_listen_socket = data_socket;
    return FTP_STATUS_ENTERING_PASSIVE;
}

void ftp_state_free(ftp_state_t *state)
{
    assert(state != NULL);

    if(state->username != NULL)
        free(state->username);
    
    if(state->password != NULL)
        free(state->password);

    if(state->curdir != NULL)
        free(state->curdir);

    free(state);
}
