#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>

#include "util.h"
#include "state.h"

ftp_state_t* ftp_state_new(void)
{
    ftp_state_t *state = malloc_e(sizeof(*state));

    state->username = NULL;
    state->password = NULL;
    state->curdir = NULL;
    state->is_passive = 0;

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

int ftp_state_set_passive(ftp_state_t *state, int is_passive)
{
    assert(state != NULL);
    
    int old = state->is_passive;
    state->is_passive = (is_passive != 0) ? 1 : 0;
    
    return old;
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
