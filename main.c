#include "engine.h"
#include "resource_db.h"
#include "mem_track.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    /* If no arguments passed, just inform the user */
    if (argc < 2) {
        printf("Smart Classroom CLI Ready. Waiting for commands from UI...\n");
        return 0;
    }

    /* Load CSV into Resource Database */
    ResourceDatabase* rdb = rdb_create(16);
    rdb_load_from_csv(rdb, "rooms.csv");

    /* Sync Database into the Reallocation Engine */
    ClassroomSystem *sys = system_create(rdb->list.size, 100); 
    ClassroomNode* curr = rdb->list.head;
    int r_idx = 0;
    while(curr) {
        system_set_room(sys, r_idx, curr->data.room_id, curr->data.capacity, curr->data.has_projector, 0, 5);
        r_idx++;
        curr = curr->next;
    }
    
    engine_set_system(sys);
    engine_set_verbose(1);

    /* Process Booking Command */
    if (strcmp(argv[1], "--book") == 0 && argc == 6) {
        int c_id = atoi(argv[2]);
        char* r_id_str = argv[3];
        int start = atoi(argv[4]);
        int end = atoi(argv[5]);

        int target_r_idx = -1;
        for(int i = 0; i < sys->n_rooms; i++) {
            if(strcmp(sys->rooms[i].room_id, r_id_str) == 0) {
                target_r_idx = i; break;
            }
        }

        system_set_class(sys, 0, c_id);
        if (target_r_idx != -1 && system_book_class(sys, c_id, target_r_idx, start, end) == 0) {
            printf("RESULT_ROOM:%s\n", r_id_str);
        } else {
            printf("RESULT_ROOM:FAILED\n");
        }
    } 
    /* Process Reallocation Command */
    else if (strcmp(argv[1], "--realloc") == 0 && argc == 7) {
        int c_id = atoi(argv[2]);
        int cap = atoi(argv[3]);
        int proj = atoi(argv[4]);
        int start = atoi(argv[5]);
        int end = atoi(argv[6]);

        system_set_class(sys, 0, c_id); 
        int res_idx = reallocate_class(c_id, cap, proj, start, end);
        if (res_idx >= 0) {
            printf("RESULT_ROOM:%s\n", sys->rooms[res_idx].room_id);
        } else {
            printf("RESULT_ROOM:FAILED\n");
        }
    }

    system_destroy(sys);
    rdb_destroy(rdb);
    return 0;
}