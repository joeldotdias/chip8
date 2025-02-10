#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>

void *c8_malloc(size_t size, const char *file, int line);
void *c8_calloc(size_t nmemb, size_t size, const char *file, int line);

#endif
