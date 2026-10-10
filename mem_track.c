#include "mem_track.h"
#include <stdio.h>
#include <stdlib.h>

static size_t g_allocs = 0, g_frees = 0;

void *tracked_malloc(size_t n) {
    void *p = malloc(n);
    if (!p) {
        fprintf(stderr, "fatal: out of memory (%zu bytes)\n", n);
        exit(EXIT_FAILURE);
    }
    g_allocs++;
    return p;
}

void tracked_free(void *p) {
    if (p) { free(p); g_frees++; }
}

size_t mem_alloc_count(void) { return g_allocs; }
size_t mem_free_count(void)  { return g_frees;  }