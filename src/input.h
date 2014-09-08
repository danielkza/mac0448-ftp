#ifndef PICOFTPD_INPUT_H_INCLUDED
#define PICOFTPD_INPUT_H_INCLUDED

#include "state.h"
#include "commands.h"

#define PICOFTPD_LINE_MAX 1024

int ftp_input_read(int socket, char *buf, size_t *len, size_t max_size);
ftp_result_t ftp_input_parse_run(char *buf, ftp_state_t *state);
void ftp_input_shift_buf(char *buf, size_t *len);


#endif
