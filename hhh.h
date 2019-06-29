#ifndef HHH_H
#define HHH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

#define strlcpy _strlcpy
void _strlcpy(char *dest, const char *src, size_t size);
void die(const char *err, ...);
void *xrealloc(void *ptr, size_t size);
char *trim(char *s);
void split(char *line, char **left, char **right, char delim);

#endif /* HHH */
