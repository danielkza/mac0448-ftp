#define _GNU_SOURCE

#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "util.h"

#include "state.h"

ftp_state_t* ftp_state_new(void)
{
    ftp_state_t *state = malloc(sizeof(*state));
    if(state == NULL)
        return NULL;

    /* Initialize defaults first so cleanup can rely on their values to
     * determine what too cleanup
     */

    state->username = NULL;
    state->password = NULL;
    state->curdir_fd = -1;

    state->is_mode_stream = 1;
    state->is_type_image = 0;
    state->is_structure_file = 1;

    state->data_listen_socket = -1;
    state->control_socket = -1;
    state->data_pid = 0;

    state->control_sem = NULL;
    state->control_sem_initialized = false;

    // We can create the complex stuff now
    state->curdir_fd = open(".", O_RDONLY);
    if(state->curdir_fd == -1) {
        ftp_debug_perror(state, "open");
        goto error;
    }

    state->control_sem = mmap(NULL, sizeof(*state->control_sem),
                              PROT_READ | PROT_WRITE,
                              MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if(state->control_sem == NULL) {
        ftp_debug_perror(state, "mmap");
        goto error;
    }

    if(sem_init(state->control_sem, 1, 1) == -1) {
        ftp_debug_perror(state, "sem_init");
        goto error;
    }

    state->control_sem_initialized = 1;
    return state;

error:
    ftp_state_free(state);
    return NULL;
}

bool ftp_state_free(ftp_state_t *state)
{
    if(state == NULL)
        return true;

    if(state->data_listen_socket != -1 || state->control_socket != -1
       || state->data_pid > 0)
        return false;

    if(state->curdir_fd != -1)
        close(state->curdir_fd);

    if(state->control_sem != NULL) {
        if(state->control_sem_initialized)
            sem_destroy(state->control_sem);

        munmap(state->control_sem, sizeof(*state->control_sem));
    }

    free(state->username);
    free(state->password);
    free(state);

    return true;
}

bool ftp_state_set_username(ftp_state_t *state, const char *username)
{
    assert(state != NULL);

    char *new_username = NULL;
    if(username != NULL && (new_username = strdup(username)) == NULL)
        return false;

    free(state->username);
    state->username = new_username;

    return true;
}

bool ftp_state_set_password(ftp_state_t *state, const char *password)
{
    assert(state != NULL);

    char *new_password = NULL;
    if(password != NULL && (new_password = strdup(password)) == NULL)
        return false;

    free(state->password);
    state->password = new_password;

    return true;
}

bool ftp_state_chdir(ftp_state_t *state, const char *dir)
{
    assert(state != NULL);
    assert(dir != NULL);

    int new_dir = openat(state->curdir_fd, dir, O_RDONLY);
    if(new_dir == -1)
        return false;

    if(fchdir(new_dir) == -1)
        return false;

    close(state->curdir_fd);
    state->curdir_fd = new_dir;

    return true;
}

