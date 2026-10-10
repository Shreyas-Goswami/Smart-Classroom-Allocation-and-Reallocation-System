#include "max_heap.h"
#include "mem_track.h"
#include <string.h>

static int outranks(const HeapItem *a, const HeapItem *b) {
    if (a->score != b->score) return a->score > b->score;
    return a->room_idx < b->room_idx;
}

static void swap_items(HeapItem *a, HeapItem *b) { HeapItem t = *a; *a = *b; *b = t; }

MaxHeap *heap_create(int initial_capacity) {
    if (initial_capacity < 4) initial_capacity = 4;
    MaxHeap *h = tracked_malloc(sizeof *h);
    h->data = tracked_malloc((size_t)initial_capacity * sizeof(HeapItem));
    h->size = 0;
    h->capacity = initial_capacity;
    return h;
}

void heap_push(MaxHeap *h, int room_idx, float score) {
    if (h->size == h->capacity) {
        int new_cap = h->capacity * 2;
        HeapItem *nd = tracked_malloc((size_t)new_cap * sizeof(HeapItem));
        memcpy(nd, h->data, (size_t)h->size * sizeof(HeapItem));
        tracked_free(h->data);
        h->data = nd;
        h->capacity = new_cap;
    }
    int i = h->size++;
    h->data[i].room_idx = room_idx;
    h->data[i].score = score;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (!outranks(&h->data[i], &h->data[p])) break;
        swap_items(&h->data[i], &h->data[p]);
        i = p;
    }
}

int heap_pop(MaxHeap *h, HeapItem *out) {
    if (h->size == 0) return 0;
    *out = h->data[0];
    h->data[0] = h->data[--h->size];
    int i = 0;
    for (;;) {
        int l = 2 * i + 1, r = l + 1, best = i;
        if (l < h->size && outranks(&h->data[l], &h->data[best])) best = l;
        if (r < h->size && outranks(&h->data[r], &h->data[best])) best = r;
        if (best == i) break;
        swap_items(&h->data[i], &h->data[best]);
        i = best;
    }
    return 1;
}

int heap_empty(const MaxHeap *h) { return h->size == 0; }

void heap_destroy(MaxHeap *h) {
    if (!h) return;
    tracked_free(h->data);
    tracked_free(h);
}