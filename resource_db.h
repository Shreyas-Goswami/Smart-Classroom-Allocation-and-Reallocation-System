#ifndef RESOURCE_DB_H
#define RESOURCE_DB_H

typedef enum { ROOM_AVAILABLE, ROOM_OCCUPIED, ROOM_MAINTENANCE } RoomStatus;

typedef struct {
    char room_id[16];
    int capacity;
    int has_projector;
    char location[64];
    RoomStatus status;
} Classroom;

typedef struct ClassroomNode {
    Classroom data;
    struct ClassroomNode* prev;
    struct ClassroomNode* next;
} ClassroomNode;

typedef struct {
    ClassroomNode* head;
    ClassroomNode* tail;
    int size;
} ClassroomList;

typedef struct HashEntry {
    char room_id[16];
    ClassroomNode* node;
    struct HashEntry* next;
} HashEntry;

typedef struct {
    HashEntry** buckets;
    int bucket_count;
    int size;
} RoomHashTable;

typedef struct {
    ClassroomList list;
    RoomHashTable index;
} ResourceDatabase;

ResourceDatabase* rdb_create(int initial_buckets);
void rdb_destroy(ResourceDatabase* db);
int rdb_add_room(ResourceDatabase* db, const char* room_id, int capacity, int has_projector, const char* location, RoomStatus status);
Classroom* rdb_search_room(ResourceDatabase* db, const char* room_id);
int rdb_remove_room(ResourceDatabase* db, const char* room_id);
int rdb_load_from_csv(ResourceDatabase* db, const char* filename);
#endif