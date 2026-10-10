#ifndef MEM_TRACK_H
#define MEM_TRACK_H
#include <stddef.h>

void  *tracked_malloc(size_t n);
void   tracked_free(void *p);
size_t mem_alloc_count(void);
size_t mem_free_count(void);

#endif