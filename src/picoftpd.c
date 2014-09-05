#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "commands.h"
#include "util.h"

static ssize_t str_remove_suffix(char *str, const char *suffix)
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

static ftp_status_t ftp_parse_run_command(FILE *in, char **buf, size_t *buf_len, ftp_state_t *state)
{
    assert(buf != NULL);
    assert(buf_len != NULL);
    
    errno = 0;
    if(getline(buf, buf_len, in) < 0) {
        if(errno != 0) {
            perror("Error: getline: ");
            return FTP_STATUS_INVALID_SYNTAX;
        }
        
        return FTP_COMMAND_EOF;
    }
   
    if(str_remove_suffix(*buf, "\r\n") <= 0)
        return FTP_STATUS_INVALID_SYNTAX;
   
    char *command_name = *buf, *arg;
    char *space_pos = strchr(*buf, ' ');
    if(space_pos == NULL)
        arg = NULL;
    else {
        *space_pos = '\0';
        arg = space_pos + 1;
    }
    
    ftp_command_t *cur_command = &(picoftpd_commands[0]);

    while(cur_command->name != NULL
          && strcasecmp(cur_command->name, command_name) != 0) {
        cur_command++;
    }
    
    if(cur_command->name == NULL)
        return FTP_STATUS_INVALID_SYNTAX;
    
    printf("arg: '%s'\n", arg);
    int command_res = (*cur_command->func)(arg, state);
    
    return command_res;
}

char* ftp_pasv_reply(ftp_state_t *state)
{
    assert(state != NULL);
    
    struct sockaddr_in data_addr;
    socklen_t data_addr_len = sizeof(data_addr);
    if(getsockname(state->data_listen_socket, (struct sockaddr *)&data_addr, &data_addr_len) == -1)
        return NULL;
    
    char *addr = strdup_e(inet_ntoa(data_addr.sin_addr)), *addr_dot = addr;
    while((addr_dot = strchr(addr_dot, '.')) != NULL)
        *(addr_dot++) = ',';
    
    unsigned int port_high = (data_addr.sin_port >> 8) & 0xFF,
                 port_low = (data_addr.sin_port) & 0xFF;
      
    char *result;
    if(asprintf(&result, "227 Entering Passive Mode (%s,%u,%u)\r\n",
                addr, port_high, port_low) == -1) {
        fputs("Out of memory, aborting!\n", stderr);
        abort();
    }
    
    free(addr);
    return result;
}

void ftp_do_control(int control_socket)
{
    ftp_state_t *state = ftp_state_new();
    if(state == NULL)
        return;
    
    state->control_socket = control_socket;
    
    FILE *control_file = fdopen(control_socket, "w+");
    if(control_file == NULL)
        return;
    
    printf("control_file: %p\n", control_file);
    
    char *buf = NULL;
    size_t buf_len = 0;
       
    for(;;) {
        ftp_status_t ftp_status;
        ftp_status = ftp_parse_run_command(control_file, &buf, &buf_len, state);
        
        printf("status %d\n", (int) ftp_status);
        
        if (ftp_status == FTP_STATUS_CLOSING)
            break;
        else if (ftp_status == FTP_STATUS_ENTERING_PASSIVE) {
            char *reply = ftp_pasv_reply(state); 
            fputs(reply, control_file);
            free(reply);
        }
        else if (ftp_status > 0)
            fprintf(control_file, "%d\r\n", (int)ftp_status);
        else
            break;
    }
    
    free(state);
    free(buf);
}

int main(int argc, char **argv)
{
    int port;
    if(argc < 2 || (port = atoi(argv[1])) <= 0) {
        puts("Error: invalid port\n");
        return 1;
    }
    
    struct sockaddr_in control_addr = {0};
    control_addr.sin_family      = AF_INET;
    control_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    control_addr.sin_port = htons(port);
    
    if(argc > 2) {
        if(inet_aton(argv[2], &(control_addr.sin_addr)) == 0) {
            puts("Error: invalid IP\n");
            return 1;
        }   
    }
            
    int control_listen_socket;
    if((control_listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket: ");
        return 1;
    }
    
    printf("socket\n");
    
    if(bind(control_listen_socket, (struct sockaddr*) &control_addr, sizeof(control_addr)) == -1) {
        perror("bind: ");
        close(control_listen_socket);
     
        return 1;
    }
    
    printf("bind\n");
    
    if(listen(control_listen_socket, 1) == -1) {
        perror("listen: ");
        close(control_listen_socket);
     
        return 1;
    }
    
    printf("listen\n");
    
    for(;;) {
        int control_socket = accept(control_listen_socket, (struct sockaddr *) NULL, NULL);
        if(control_socket == -1) {
            perror("accept: ");
            continue;
        }
        
        printf("accept %d\n", control_socket);
        
        pid_t child = fork();
        if(child < 0) {
            perror("fork: ");
            abort();
        }
            
        if(child == 0) {
            close(control_listen_socket);
            printf("do_control\n");
            ftp_do_control(control_socket);
        }
        
        close(control_socket);
    }
    
    close(control_listen_socket);
    
    return 0;
}