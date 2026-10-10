#include "engine.h"
#include "ai_model.h"
#include "max_heap.h"
#include "mem_track.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ClassroomSystem *g_sys = NULL;
static int g_verbose = 0;

void engine_set_system(ClassroomSystem *sys) { g_sys = sys; }
void engine_set_verbose(int on)              { g_verbose = on; }

ClassroomSystem *system_create(int n_rooms, int n_classes) {
    ClassroomSystem *s = tracked_malloc(sizeof *s);
    s->n_rooms = n_rooms;     
    s->rooms   = tracked_malloc((size_t)n_rooms   * sizeof(Room));
    s->n_classes = n_classes; 
    s->classes = tracked_malloc((size_t)n_classes * sizeof(ClassAlloc));
    for (int i = 0; i < n_rooms; i++) s->rooms[i] = (Room){"", 0, 0, 0, 0, NULL};
    for (int i = 0; i < n_classes; i++) s->classes[i] = (ClassAlloc){0, 0, -1, 0, 0, NULL, 0};
    return s;
}

static void slots_free(int **slots, int n_slots) {
    if (!slots) return;
    for (int i = 0; i < n_slots; i++) tracked_free(slots[i]);
    tracked_free(slots);
}

static int **slots_alloc(int start, int end, int *n_out) {
    int n = (end - start + SLOT_LEN - 1) / SLOT_LEN;
    int **g = tracked_malloc((size_t)n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        g[i] = tracked_malloc(2 * sizeof(int));
        g[i][0] = start + i * SLOT_LEN;
        g[i][1] = (g[i][0] + SLOT_LEN < end) ? g[i][0] + SLOT_LEN : end;
    }
    *n_out = n;
    return g;
}

void system_destroy(ClassroomSystem *sys) {
    if (!sys) return;
    for (int i = 0; i < sys->n_rooms; i++)   interval_free_tree(sys->rooms[i].bookings);
    for (int i = 0; i < sys->n_classes; i++) slots_free(sys->classes[i].slots, sys->classes[i].n_slots);
    tracked_free(sys->rooms);
    tracked_free(sys->classes);
    tracked_free(sys);
}

void system_set_room(ClassroomSystem *sys, int idx, const char* room_id, int capacity, int has_projector, int location, int conflict_rate) {
    Room *r = &sys->rooms[idx];
    strncpy(r->room_id, room_id, 15);
    r->room_id[15] = '\0';
    r->capacity = capacity;
    r->has_projector = has_projector;
    r->location = location;
    r->conflict_rate = conflict_rate;
}

void system_set_class(ClassroomSystem *sys, int idx, int class_id) {
    sys->classes[idx].class_id = class_id;
}

static int find_class_idx(const ClassroomSystem *s, int class_id) {
    for (int i = 0; i < s->n_classes; i++)
        if (s->classes[i].class_id == class_id) return i;
    return -1;
}

static int book(ClassroomSystem *s, int cidx, int ridx, int start, int end) {
    Room *r = &s->rooms[ridx];
    ClassAlloc *c = &s->classes[cidx];
    if (start >= end || (end - start + SLOT_LEN - 1) / SLOT_LEN > MAX_SLOTS) return -1;
    if (interval_search_overlap(r->bookings, start, end)) return -1;   

    c->slots = slots_alloc(start, end, &c->n_slots);
    r->bookings = interval_insert(r->bookings, start, end, c->class_id);
    c->room_idx = ridx; c->start_time = start; c->end_time = end; c->active = 1;
    return 0;
}

static void unbook(ClassroomSystem *s, int cidx) {
    ClassAlloc *c = &s->classes[cidx];
    if (!c->active) return;
    int deleted = 0;
    Room *r = &s->rooms[c->room_idx];
    r->bookings = interval_delete(r->bookings, c->start_time, c->class_id, &deleted);
    slots_free(c->slots, c->n_slots);           
    c->slots = NULL; c->n_slots = 0; c->active = 0;
}

int system_book_class(ClassroomSystem *sys, int class_id, int room_idx, int start, int end) {
    int cidx = find_class_idx(sys, class_id);
    if (cidx < 0 || room_idx < 0 || room_idx >= sys->n_rooms || sys->classes[cidx].active) return -1;
    return book(sys, cidx, room_idx, start, end);
}

int reallocate_class(int class_id, int required_capacity, int need_projector, int start_time, int end_time) {
    ClassroomSystem *s = g_sys;
    if (!s || required_capacity <= 0 || start_time >= end_time) return -1;

    int cidx = find_class_idx(s, class_id);
    if (cidx < 0) return -1;
    ClassAlloc *c = &s->classes[cidx];

    int was_active = c->active;
    int old_room = c->room_idx, old_start = c->start_time, old_end = c->end_time;
    int old_loc = was_active ? s->rooms[old_room].location : 0;

    unbook(s, cidx);

    MaxHeap *heap = heap_create(s->n_rooms);
    for (int i = 0; i < s->n_rooms; i++) {
        const Room *r = &s->rooms[i];
        if (was_active && i == old_room) continue;            
        if (r->capacity < required_capacity) continue;        
        if (need_projector && !r->has_projector) continue; /* PROJECTOR FILTER APPLIED */
        if (interval_search_overlap(r->bookings, start_time, end_time)) continue; 

        int cap_diff = r->capacity - required_capacity;
        int dist     = abs(r->location - old_loc);
        float score  = calculate_ai_score(cap_diff, dist, r->conflict_rate);
        heap_push(heap, i, score);
    }

    HeapItem best;
    int result_idx = -1;
    if (heap_pop(heap, &best)) {
        if (book(s, cidx, best.room_idx, start_time, end_time) == 0) {
            result_idx = best.room_idx;
            if (g_verbose) {
                printf("  [engine] class %d -> room %s (AI score %.4f)\n", class_id, s->rooms[best.room_idx].room_id, best.score);
                HeapItem alt; int rank = 2;               
                while (heap_pop(heap, &alt))
                    printf("  [engine]   runner-up #%d: room %s (score %.4f)\n", rank++, s->rooms[alt.room_idx].room_id, alt.score);
            }
        }
    }
    heap_destroy(heap);

    if (result_idx < 0 && was_active) {
        book(s, cidx, old_room, old_start, old_end);
        if (g_verbose) printf("  [engine] no alternative; class %d restored to room %s\n", class_id, s->rooms[old_room].room_id);
    }
    return result_idx; 
}