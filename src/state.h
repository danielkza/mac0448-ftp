#ifndef STATE_H
#define STATE_H

typedef struct {
    char *username;
    char *password;

    char *curdir;
} ftp_state_t;

ftp_state_t* ftp_state_new(void);
void ftp_state_set_username(ftp_state_t *, const char *);
void ftp_state_set_password(ftp_state_t *, const char *);
int ftp_state_set_curdir(ftp_state_t *, const char *);
void ftp_state_free(ftp_state_t *);

#endif