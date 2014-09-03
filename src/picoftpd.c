#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

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

int main(UNUSED int argc, UNUSED char **argv)
{
    int ret = 0;

    char *buf = NULL;
    size_t buf_len = 0;

    ftp_state_t *state = ftp_state_new();

    while(1) {
        ftp_status_t ftp_status;
        ftp_status = ftp_parse_run_command(stdin, &buf, &buf_len, state);
        if (ftp_status == FTP_STATUS_CLOSING)
            break;
        else if (ftp_status > 0)
            printf("%d\n", (int)ftp_status);
        else
            break;
        
        
    }
    
    free(state);
    free(buf);
    return ret;
}
