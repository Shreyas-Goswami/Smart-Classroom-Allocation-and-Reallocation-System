#ifndef ENGINE_H
#define ENGINE_H

#include "interval_tree.h"

#define SLOT_LEN   1      
#define MAX_SLOTS  1024   

typedef struct {
    char room_id[16];           /* String to support IDs like A101 */
    int capacity;               
    int has_projector;          /* Projector support */
    int location;               
    int conflict_rate;          
    IntervalNode *bookings;     
} Room;

typedef struct {
    int class_id;
    int active;                 
    int room_idx;               
    int start_time, end_time;   
    int **slots;                
    int n_slots;
} ClassAlloc;

typedef struct {
    Room *rooms;       int n_rooms;
    ClassAlloc *classes; int n_classes;
} ClassroomSystem;

ClassroomSystem *system_create(int n_rooms, int n_classes);
void system_destroy(ClassroomSystem *sys);
void system_set_room(ClassroomSystem *sys, int idx, const char* room_id, int capacity, int has_projector, int location, int conflict_rate);
void system_set_class(ClassroomSystem *sys, int idx, int class_id);
int system_book_class(ClassroomSystem *sys, int class_id, int room_idx, int start, int end);

void engine_set_system(ClassroomSystem *sys);   
void engine_set_verbose(int on);                

/* Updated signature to include need_projector */
int reallocate_class(int class_id, int required_capacity, int need_projector, int start_time, int end_time);

#endif