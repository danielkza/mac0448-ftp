#ifndef PICOFTPD_UTIL_H_INCLUDED
#define PICOFTPD_UTIL_H_INCLUDED

#include <stddef.h>

#define UNUSED __attribute__((unused))

char *strdup_e(const char *);
void *malloc_e(size_t);

#endif