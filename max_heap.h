#ifndef MAX_HEAP_H
#define MAX_HEAP_H

typedef struct {
    int   room_idx;
    float score;
} HeapItem;

typedef struct {
    HeapItem *data;
    int size;
    int capacity;
} MaxHeap;

MaxHeap *heap_create(int initial_capacity);
void     heap_push(MaxHeap *h, int room_idx, float score);
int      heap_pop(MaxHeap *h, HeapItem *out);
int      heap_empty(const MaxHeap *h);
void     heap_destroy(MaxHeap *h);

#endif