#define _GNU_SOURCE

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include <unistd.h>
#include <strings.h>
#include <signal.h>
#include <semaphore.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "state.h"
#include "util.h"

#include "commands.h"

ftp_result_t ftp_write_response(ftp_state_t *state, ftp_status_t code,
                                const char *format, ...)
{
    assert(state != NULL);

    if(state->control_socket == -1 || !state->control_sem_initialized) {
        errno = EINVAL;
        return FTP_RESULT_ABORT;
    }

    ftp_debug(state, "write_response waiting (%d)\n", (int)getpid());

    while(sem_wait(state->control_sem) == -1) {
        if(errno != EAGAIN) {
            ftp_debug_perror(state, "sem_wait");
            return FTP_RESULT_ABORT;
        }
    }

    ftp_debug(state, "write_response started (%d)\n", (int)getpid());

    ftp_status_t result = FTP_RESULT_ABORT;

    if(dprintf(state->control_socket, "%u ", (unsigned int)code) < 0)
        goto cleanup;

    va_list args;
    va_start(args, format);

    ssize_t len = vdprintf(state->control_socket, format, args);
    va_end(args);

    if(len < 0)
        goto cleanup;

    if(write(state->control_socket, "\r\n", 2) < 0)
        goto cleanup;

    result = FTP_RESULT_OK;

cleanup:
    sem_post(state->control_sem);
    ftp_debug(state, "write_response done (%d)\n", (int)getpid());
    return result;
}

ftp_result_t ftp_hello(ftp_state_t *state)
{
    return ftp_write_response(state, FTP_STATUS_OK, "picoftpd");
}

ftp_result_t ftp_syst(UNUSED const char *args, ftp_state_t *state)
{
    return ftp_write_response(state, FTP_STATUS_SYSTEM_NAME, "UNIX Type: L8");
}

ftp_result_t ftp_noop(UNUSED const char *arg, ftp_state_t *state)
{
    return ftp_write_response(state, FTP_STATUS_OK, "Command OK");
}

ftp_result_t ftp_quit(UNUSED const char *arg, ftp_state_t *state)
{
    return ftp_write_response(state, FTP_STATUS_CLOSING, "Goodbye");
}

ftp_result_t ftp_abor(UNUSED const char *arg, ftp_state_t *state)
{
    if(state->data_pid > 0) {
        return FTP_RESULT_ABORT_TRANSFER;
    } else {
        if(state->data_listen_socket != -1) {
            close(state->data_listen_socket);
            state->data_listen_socket = -1;
        }

        return ftp_write_response(state, FTP_STATUS_TRANSFER_SUCCEEDED,
                                  "Data connection closed");
    }
}



ftp_result_t ftp_user(const char *arg, ftp_state_t *state)
{
    ftp_debug(state, "User: '%s'\n", arg);
    ftp_state_set_username(state, arg);
    ftp_state_set_password(state, NULL);

    return ftp_write_response(state, FTP_STATUS_USER_LOGGED_IN,
                              "Logged in as '%s'", state->username);
}

ftp_result_t ftp_pass(const char *arg, ftp_state_t *state)
{
    if(state->username == NULL)
        return ftp_write_response(state, FTP_STATUS_INVALID_COMMAND_SEQUENCE,
                                  "Cannot set password before USER");

    ftp_state_set_password(state, arg);
    return ftp_write_response(state, FTP_STATUS_USER_LOGGED_IN,
        "Password ignored, logged in as '%s'", state->username);
}

ftp_result_t ftp_type(const char *arg, ftp_state_t *state)
{
    /* Check for valid types. The only one we understand and accept is 'I'.
     * Anything else that starts with 'A', 'E' or 'L', and is followed by either
     * a space or the end of string, is understood, but not accepted.
     * Anything else is a syntax error.
     */

    if(strcasecmp(arg, "i") == 0) {
        state->is_type_image = true;
        return ftp_write_response(state, FTP_STATUS_OK, "Type set to I");
    } else if (strchr("aAeElL", arg[0]) != NULL
               && (arg[1] == ' ' || arg[1] == '\0')) {
        return ftp_write_response(state, FTP_STATUS_PARAMETER_NOT_IMPLEMENTED,
                                  "Types other than I not implemented");
    } else {
        return ftp_write_response(state, FTP_STATUS_INVALID_SYNTAX_PARAMS,
                                  "Invalid type");
    }
}

ftp_result_t ftp_mode(const char *arg, ftp_state_t *state)
{
    if(strcasecmp(arg, "s") == 0) {
        state->is_mode_stream = true;
        return ftp_write_response(state, FTP_STATUS_OK, "Mode set to S");
    } else if (strcasecmp(arg, "b") == 0 || strcasecmp(arg, "c") == 0) {
        return ftp_write_response(state, FTP_STATUS_PARAMETER_NOT_IMPLEMENTED,
                                  "Modes other than S not implemented");
    } else {
        return ftp_write_response(state, FTP_STATUS_INVALID_SYNTAX_PARAMS,
                                  "Invalid mode");
    }
}

ftp_result_t ftp_stru(const char *arg, ftp_state_t *state)
{
    if(strcasecmp(arg, "f") == 0) {
        state->is_structure_file = true;
        return ftp_write_response(state, FTP_STATUS_OK, "Structure set to F");
    } else if (strcasecmp(arg, "p") == 0 || strcasecmp(arg, "r") == 0) {
        return ftp_write_response(state, FTP_STATUS_PARAMETER_NOT_IMPLEMENTED,
                                  "Structures other than F not implemented");
    } else {
        return ftp_write_response(state, FTP_STATUS_INVALID_SYNTAX_PARAMS,
                                  "Invalid structure");
    }
}

static ftp_result_t ftp_write_pasv_response(ftp_state_t *state)
{
    assert(state != NULL);

    if(state->data_listen_socket == -1) {
        ftp_debug(state, "PASV: failed to open data socket\n");
        goto error;
    }

    struct sockaddr_in data_addr;
    socklen_t data_addr_len = sizeof(data_addr);
    if(getsockname(state->data_listen_socket, (struct sockaddr *)&data_addr,
                   &data_addr_len) == -1) {
        ftp_debug_perror(state, "getsockname");
        goto error;
    }

    char *addr = strdup(inet_ntoa(data_addr.sin_addr)), *addr_dot = addr;
    if(addr == NULL) {
        ftp_debug_perror(state, "strdup");
        goto error;
    }

    while((addr_dot = strchr(addr_dot, '.')) != NULL)
        *(addr_dot++) = ',';

    uint16_t port = ntohs(data_addr.sin_port);
    unsigned int port_high = (port >> 8) & 0xFF, port_low = port & 0xFF;

    return ftp_write_response(state, FTP_STATUS_ENTERING_PASSIVE,
                              "Entering Passive Mode (%s,%u,%u)",
                              addr, port_high, port_low);

error:
    /* Close the socket, since even though it was succesfully open, nobody
     * will ever know about it, considering the response failed
     */
    if(state->data_listen_socket != -1) {
        close(state->data_listen_socket);
        state->data_listen_socket = -1;
    }

    return ftp_write_response(state, FTP_STATUS_CANT_OPEN_DATA_CONNECTION,
                              "Failed to open new data port");
}

ftp_result_t ftp_pasv(UNUSED const char *arg, ftp_state_t *state)
{
    assert(state != NULL);

    if(state->data_listen_socket != -1) {
        if(state->data_pid > 0) {
            ftp_debug(state, "PASV: attempted while transfer in progress\n");

            return ftp_write_response(state,
                FTP_STATUS_INVALID_COMMAND_SEQUENCE,
                "Transfer currently in progress, cannot switch data port");
        }

        ftp_debug(state, "PASV: closing existing data connection\n");

        close(state->data_listen_socket);
        state->data_listen_socket = -1;
    }

    ftp_debug(state, "PASV: Opening data socket\n");

    /* Read the socket parameters from the existing control socket so we
     * bind to the same address we're talking to. Just change the port to an
     * automatically assigned one.
     */

    int data_listen_socket = -1;

    struct sockaddr_in data_addr = {0};
    socklen_t data_addr_len = sizeof(data_addr);
    if(getsockname(state->control_socket, (struct sockaddr *)&data_addr,
                   &data_addr_len) == -1) {
        ftp_debug_perror(state, "getsockname");
        goto error;
    }

    data_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(data_listen_socket == -1) {
        ftp_debug_perror(state, "socket");
        goto error;
    }

    data_addr.sin_port = htons(0);

    if(bind(data_listen_socket, (struct sockaddr *)&data_addr,
            sizeof(data_addr)) == -1) {
        ftp_debug_perror(state, "bind");
        goto error;
    }

    if(listen(data_listen_socket, 1) == -1) {
        ftp_debug_perror(state, "listen");
        goto error;
    }

    ftp_debug(state, "PASV: data socket opened succesfully\n");

    state->data_listen_socket = data_listen_socket;
    return ftp_write_pasv_response(state);

error:
    ftp_debug(state, "PASV: failed to open data socket\n");

    if(data_listen_socket != -1)
        close(data_listen_socket);

    return ftp_write_pasv_response(state);
}

static inline bool ftp_transfer_read_loop(FILE *read_file, int data_socket)
{
    char c;
    for(;;) {
        int ic = fgetc(read_file);
        if(ic == EOF) {
            if(!ferror(read_file))
                return true;

            break;
        }

        c = (char)ic;
        if(write(data_socket, &c, 1) == -1)
            break;
    }

    return false;
}

static inline bool ftp_transfer_write_loop(FILE *write_file, int data_socket)
{
    char c;
    for(;;) {
        ssize_t len = read(data_socket, &c, 1);
        if(len > 0) {
            if(fputc(c, write_file) == EOF) {
                break;
            }
        } else if(len == 0) {
            return true;
        } else {
            break;
        }
    }

    return false;
}

ftp_result_t ftp_start_transfer(ftp_state_t *state, FILE *read_file,
                                FILE *write_file)
{
    const char *err_msg = NULL;
    if(!state->is_type_image)
        err_msg = "Invalid transfer type set";
    else if(!state->is_mode_stream)
        err_msg = "Invalid transfer mode set";
    else if(!state->is_structure_file)
        err_msg = "Invalid transfer structure set";
    else if(state->data_listen_socket == -1)
        err_msg = "Passive data connection not started";
    else if(state->data_pid > 0)
        err_msg = "Another transfer already in progress";

    if(err_msg != NULL) {
        return ftp_write_response(state, FTP_STATUS_INVALID_COMMAND_SEQUENCE,
                                  "%s", err_msg);
    }

    // read_file and write_file can't be both set or both null.
    assert((read_file != NULL) ^ (write_file != NULL));

    ftp_debug(state, "transfer: blocking SIGCHILD\n");

    sigset_t sig_block_set;
    sigemptyset(&sig_block_set);
    sigaddset(&sig_block_set, SIGCHLD);

    if(sigprocmask(SIG_BLOCK, &sig_block_set, NULL) == -1) {
        ftp_debug_perror(state, "sigprocmask");
        return FTP_RESULT_ABORT;
    }

    ftp_debug(state, "transfer: forking\n");
    if((state->data_pid = fork()) == -1) {
        ftp_debug_perror(state, "fork");
        return ftp_write_response(state, FTP_STATUS_LOCAL_ERROR,
                                  "Transfer failed due to system error");
    }

    // child
    if(state->data_pid == 0) {
        bool ret = false;

        int data_socket = accept(state->data_listen_socket, NULL, NULL);
        if(data_socket == -1) {
            ftp_debug_perror(state, "accept");
            (void)ftp_write_response(state, FTP_STATUS_CANT_OPEN_DATA_CONNECTION,
                                     "Failed waiting for data connection");
            goto cleanup;
        }

        ftp_debug(state, "transfer: accepted pid %d\n", (int)getpid());
        (void)ftp_write_response(state, FTP_STATUS_TRANSFER_STARTING,
                                 "Data transfer starting");

        if(read_file != NULL)
            ret = ftp_transfer_read_loop(read_file, data_socket);
        else
            ret = ftp_transfer_write_loop(write_file, data_socket);

        close(data_socket);

    cleanup:
        if(read_file != NULL)
            fclose(read_file);
        else
            fclose(write_file);

        ftp_debug(state, "transfer: done %d\n", (int)ret);

        exit(ret ? 0 : 1);
    }

    ftp_debug(state, "transfer: parent, closing fds\n");

    close(state->data_listen_socket);
    state->data_listen_socket = -1;

    return true;
}

ftp_result_t ftp_retr(const char *arg, ftp_state_t *state)
{
    FILE *file = fopen(arg, "rb");
    if(file == NULL) {
        return ftp_write_response(state, FTP_STATUS_PERMANENT_FILE_UNAVAILABLE,
                                  "Failed to open file: %s", strerror(errno));
    }

    ftp_start_transfer(state, file, NULL);
    fclose(file);

    return FTP_RESULT_OK;
}

ftp_result_t ftp_stor(const char *arg, ftp_state_t *state)
{
    FILE *file = fopen(arg, "wb");
    if(file == NULL) {
        return ftp_write_response(state, FTP_STATUS_PERMANENT_FILE_UNAVAILABLE,
                                  "Failed to open file: %s", strerror(errno));
    }

    ftp_start_transfer(state, NULL, file);
    fclose(file);

    return FTP_RESULT_OK;
}

const ftp_command_t picoftpd_commands[] = {
    {"NOOP", &ftp_noop, false},
    {"USER", &ftp_user, true},
    {"PASS", &ftp_pass, true},
    {"TYPE", &ftp_type, true},
    {"MODE", &ftp_mode, true},
    {"STRU", &ftp_stru, true},
    {"QUIT", &ftp_quit, false},
    {"ABOR", &ftp_abor, false},
    {"ABOR", &ftp_abor, false},
    {"PASV", &ftp_pasv, false},
    {"RETR", &ftp_retr, true},
    {"STOR", &ftp_stor, true},
    {"SYST", &ftp_syst, false},
    {NULL, NULL, false},
};

