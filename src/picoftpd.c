#define _GNU_SOURCE

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include "util.h"
#include "input.h"
#include "commands.h"

static const int CONTROL_POLL_TIMEOUT = 10; // 10ms
static const unsigned int TRANSFER_KILL_WAIT_TIME = 1; // 1s

static int kill_child(pid_t pid, unsigned int timeout)
{
    if(kill(pid, SIGTERM) == -1)
        return -1;

    alarm(timeout);

    int status = 0;
    if(waitpid(pid, &status, 0) == -1) {
        if(errno == EINTR)
            return kill(pid, SIGKILL);

        return -1;
    }

    return 0;
}

#define CHECK_RESPONSE(status, msg) \
do { \
    ftp_result_t res = ftp_write_response(state, (status), (msg)); \
    if(res != FTP_RESULT_OK) { goto abort; } \
} while(0)

static ftp_result_t check_data_child(ftp_state_t *state,
                                     ftp_result_t cmd_res)
{
    if(state->data_pid <= 0)
        return FTP_RESULT_OK;

    if(cmd_res == FTP_RESULT_ABORT || cmd_res == FTP_RESULT_ABORT_TRANSFER) {
        (void)kill_child(state->data_pid, TRANSFER_KILL_WAIT_TIME);

        CHECK_RESPONSE(FTP_STATUS_TRANSFER_FAILED, "Transfer aborted");
        CHECK_RESPONSE(FTP_STATUS_OK, "Abort successfull");
    } else {
        int status = 0;
        if(waitpid(state->data_pid, &status, WNOHANG) == state->data_pid
           && WIFEXITED(status)) {
            if(WEXITSTATUS(status) == 0) {
                CHECK_RESPONSE(FTP_STATUS_TRANSFER_SUCCEEDED,
                               "Transfer succeeded");
            } else {
                CHECK_RESPONSE(FTP_STATUS_TRANSFER_FAILED, "Transfer failed");
            }

            state->data_pid = 0;
        }
    }

    return cmd_res;
abort:
    return FTP_RESULT_ABORT;
}

#undef CHECK_RESPONSE

static void ftp_do_control(int control_socket)
{
    ftp_state_t *state = ftp_state_new();
    if(state == NULL)
        return;

    state->control_socket = control_socket;
    if(ftp_hello(state) != FTP_RESULT_OK)
        goto cleanup;

    char line[PICOFTPD_LINE_MAX] = "";
    size_t line_len = 0;

    struct pollfd control_poll;
    control_poll.fd = control_socket;
    control_poll.events = POLLIN | POLLERR;

    for(;;) {
        /* Happily continue running if we get OK results, or if no commands are
         * read from the control socket
         */
        ftp_result_t res = FTP_RESULT_OK;

        control_poll.revents = 0;

        int poll_res = poll(&control_poll, 1, CONTROL_POLL_TIMEOUT);
        if(poll_res == -1) {
            ftp_debug_perror(state, "poll");
            res = FTP_RESULT_ABORT;
        } else if(poll_res > 0) {
            int read_res = ftp_input_read(control_socket, line, &line_len,
                                          sizeof(line));
            if(read_res == -1) {
                ftp_debug_perror(state, "recv");
                res = FTP_RESULT_ABORT;
            } else if(read_res > 0) {
                res = ftp_input_parse_run(line, state);
                ftp_input_shift_buf(line, &line_len);
            }
        }

        res = check_data_child(state, res);

        if(res == FTP_RESULT_EOF) {
            ftp_quit(NULL, state);
            break;
        } else if(res == FTP_RESULT_ABORT) {
            break;
        }
    }

cleanup:
    close(control_socket);
    state->control_socket = -1;

    ftp_state_free(state);
}

int main(int argc, char **argv)
{
    uint16_t port;
    if(argc < 2 || (port = atoi(argv[1])) <= 0) {
        fputs("Error: invalid port\n", stderr);
        return 1;
    }

    struct sockaddr_in control_addr = {0};
    control_addr.sin_family      = AF_INET;
    control_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    control_addr.sin_port = htons(port);

    const char *ip = "0.0.0.0";
    if(argc > 2) {
        ip = argv[2];
        if(inet_aton(ip, &(control_addr.sin_addr)) == 0) {
            fputs("Error: invalid IP\n", stderr);
            return 1;
        }
    }

    int control_listen_socket;
    if((control_listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 1;
    }

    fputs("Opened main socket\n", stderr);

    if(bind(control_listen_socket, (const struct sockaddr*) &control_addr,
            sizeof(control_addr)) == -1) {
        perror("bind");
        close(control_listen_socket);

        return 1;
    }

    fputs("Bound main socket\n", stderr);

    if(listen(control_listen_socket, 1) == -1) {
        perror("listen");
        close(control_listen_socket);

        return 1;
    }

    fprintf(stderr, "Listening at %s:%u\n", ip, (unsigned int)port);

    for(;;) {
        int control_socket = accept(control_listen_socket, (struct sockaddr *) NULL, NULL);
        if(control_socket == -1) {
            perror("accept");
            continue;
        }

        fprintf(stderr, "accept %d\n", control_socket);

        pid_t child = fork();
        if(child < 0) {
            perror("fork");
        }

        if(child == 0) {
            close(control_listen_socket);
            fputs("do_control\n", stderr);
            ftp_do_control(control_socket);

            exit(0);
        } else {
            close(control_socket);
        }
    }

    close(control_listen_socket);
    return 0;
}
