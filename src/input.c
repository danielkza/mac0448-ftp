#define _GNU_SOURCE

#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include <poll.h>
#include <sys/socket.h>
#include <arpa/telnet.h>

#include "util.h"
#include "commands.h"

#include "input.h"

int ftp_input_read(int socket, char *buf, size_t *len, size_t max_size)
{
    assert(buf != NULL);
    assert(len != NULL);
    assert(max_size > 0);

    size_t max_read_len = max_size - *len - 1;
    if(max_read_len == 0) {
        *len = 0;
        errno = ERANGE;
        return -1;
    }

    ssize_t read_len = recv(socket, buf + *len, max_read_len, MSG_DONTWAIT);
    if(read_len < 0) {
        if(errno == EWOULDBLOCK)
            return 0;

        return -1;
    } else if(read_len == 0) {
        return 0;
    }

    *len += read_len;
    buf[*len] = '\0';

    size_t lines = 0;
    char *term_pos;
    while((term_pos = strstr(buf, "\r\n")) != NULL) {
        *term_pos = '\0';
        *(++term_pos) = '\0';

        // Skip telnet control characters
        unsigned char c;
        while((c = *(++term_pos)) >= TELCMD_FIRST)
            *term_pos = '\0';

        ++lines;
    }

    return lines;
}

void ftp_input_shift_buf(char *buf, size_t *len)
{
    assert(buf != NULL);
    assert(len != NULL);

    char *next_start = memmem(buf, *len, "\0", 2), *buf_end = buf + *len + 1;
    assert(next_start != NULL);

    do {
        ++next_start;
    } while(next_start < buf_end && *next_start == '\0');

    size_t copy_len = buf_end - next_start;
    if(copy_len == 0) {
        *buf = '\0';
        *len = 0;
    } else {
        memmove(buf, next_start, copy_len);
        *len = copy_len;
    }
}

ftp_result_t ftp_input_parse_run(char *buf, ftp_state_t *state)
{
    char *command_name = buf, *arg;
    char *space_pos = strchr(buf, ' ');
    if(space_pos == NULL) {
        arg = NULL;
    } else {
        *space_pos = '\0';
        arg = space_pos + 1;
    }

    ftp_debug(state, "Command: '%s'\n", command_name);

    const ftp_command_t *cur_command = &(picoftpd_commands[0]);
    while(cur_command->name != NULL
        && strcasecmp(cur_command->name, command_name) != 0) {
        ++cur_command;
        }

    if(cur_command->name == NULL)
        goto error;

    ftp_debug(state, "Argument: '%s'\n", arg);
    return (*cur_command->func)(arg, state);

error:
    return ftp_write_response(state, FTP_STATUS_INVALID_SYNTAX, "Syntax error");
}
