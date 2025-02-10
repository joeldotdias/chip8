#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>

void *c8_malloc(size_t size, const char *file, int line);
void *c8_calloc(size_t nmemb, size_t size, const char *file, int line);

#define INFO(fmt, ...)                                                                   \
    if(dbg) {                                                                            \
        printf("\033[32m" fmt "\033[0m\n", ##__VA_ARGS__);                               \
    }

#define UNIMPLEMENTED(fmt, ...)                                                          \
    do {                                                                                 \
        printf("\033[33mUNIMPLEMENTED: " fmt "\033[0m at %s:%d in %s\n", ##__VA_ARGS__,  \
               __FILE__, __LINE__, __func__);                                            \
    } while(0)

#endif
