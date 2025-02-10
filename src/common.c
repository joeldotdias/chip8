#include "common.h"
#include <stdio.h>

void *c8_malloc(size_t size, const char *file, int line) {
    void *ptr = malloc(size);
    if(!ptr) {
        fprintf(stderr, "Couldn't allocate memory in %s at line %d\n", file, line);
        exit(1);
    }
    return ptr;
}

void *c8_calloc(size_t nmemb, size_t size, const char *file, int line) {
    void *ptr = calloc(nmemb, size);
    if(!ptr) {
        fprintf(stderr, "Couldn't allocate memory in %s at line %d\n", file, line);
        exit(1);
    }
    return ptr;
}
