#ifndef PICOFTPD_STATE_H_INCLUDED
#define PICOFTPD_STATE_H_INCLUDED

typedef struct {
    char *username;
    char *password;

    char *curdir;
    int is_passive;
    int is_type_image;
    int is_mode_stream;
    int is_structure_file;
} ftp_state_t;

ftp_state_t* ftp_state_new(void);
void ftp_state_set_username(ftp_state_t *, const char *);
void ftp_state_set_password(ftp_state_t *, const char *);
int ftp_state_set_curdir(ftp_state_t *, const char *);
int ftp_state_set_passive(ftp_state_t *, int);
void ftp_state_free(ftp_state_t *);
int ftp_state_set_type_image(ftp_state_t *, int);
int ftp_state_set_mode_stream(ftp_state_t *, int);
int ftp_state_set_structure_file(ftp_state_t *, int);

#endif