#ifndef PICOFTPD_STATE_H_INCLUDED
#define PICOFTPD_STATE_H_INCLUDED

#include <stdbool.h>
#include <sys/types.h>
#include <semaphore.h>

enum {
    FTP_STATE_SEM_CONTROL = 0
} ftp_state_sem_index_t ;

typedef struct {
    char *username;
    char *password;
    int curdir_fd;

    bool is_type_image;
    bool is_mode_stream;
    bool is_structure_file;

    int control_socket;
    int data_listen_socket;
    pid_t data_pid;

    sem_t *control_sem;
    bool control_sem_initialized;
} ftp_state_t;

ftp_state_t* ftp_state_new(void);
bool ftp_state_free(ftp_state_t * state);

bool ftp_state_set_username(ftp_state_t *state, const char *username);
bool ftp_state_set_password(ftp_state_t * state, const char *password);
bool ftp_state_set_curdir(ftp_state_t *state, const char *dir);

#endif
