#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <getopt.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "picoftpd.h"

int ftp_nop(const char *arg, ftp_state_t *state)
{
    printf("NOP\n");
    printf("%s\n", arg);
    if(state != NULL) {
        printf("%s", state->username);
    }

    return 0;
}

int ftp_user(const char *arg, ftp_state_t *state)
{
    const char *user = arg;
    if(strlen(user) == 0)
        user = NULL;

    printf("Setting user to: %s\n", user != NULL ? user : "NULL");
    ftp_state_set_username(state, user);
    ftp_state_set_password(state, NULL);

    return 230;
}

int ftp_pass(const char *arg, ftp_state_t *state)
{
    const char *pass = arg;
    if(strlen(pass) == 0)
        pass = NULL;

    if(state->username == NULL)
        return 503;

    ftp_state_set_password(state, pass);
    return 230;
}

int main(__attribute__((unused)) int argc, __attribute__((unused)) char **argv)
{
    int ret = 0;
    ftp_command_t commands[] = {
        {"USER", &ftp_user},
        {"PASS", &ftp_pass},
    };

    char *buf = NULL;
    size_t buf_len = 0;

    ftp_state_t *state = ftp_state_new();

    for(;;) {
        errno = 0;
        
        if(getline(&buf, &buf_len, stdin) < 0) {
            if(errno != 0) {
                perror("Error: getline: ");
                ret = 1;
            }
    
            break;   
        }

        char *space_pos = strchr(buf, ' ');
        if(space_pos == NULL) {
            printf("invalid syntax\n");
            ret = 1;
            break;
        }

        *space_pos = '\0';
        char *command_name = buf, *arg = space_pos + 1;

        ftp_command_t *command = NULL;
        for(size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
            if(strcasecmp(commands[i].name, command_name) == 0) {
                commands = (*commands[i].func)(arg, NULL);
                goto next;
            }
        }

        printf("Unknown command %s\n", name);
    }

end:
    if(buf != NULL)
        free(buf);

    free(state);
    return ret;
}
