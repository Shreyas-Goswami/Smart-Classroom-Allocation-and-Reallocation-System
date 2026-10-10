#include "resource_db.h"
#include "mem_track.h"
#include <string.h>

static unsigned int hash_string(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

ResourceDatabase* rdb_create(int initial_buckets) {
    ResourceDatabase* db = (ResourceDatabase*)tracked_malloc(sizeof(ResourceDatabase));
    db->list.head = db->list.tail = NULL;
    db->list.size = 0;
    
    db->index.bucket_count = initial_buckets < 4 ? 4 : initial_buckets;
    db->index.size = 0;
    db->index.buckets = (HashEntry**)tracked_malloc(db->index.bucket_count * sizeof(HashEntry*));
    for (int i = 0; i < db->index.bucket_count; i++) db->index.buckets[i] = NULL;
    return db;
}

int rdb_add_room(ResourceDatabase* db, const char* room_id, int capacity, int has_projector, const char* location, RoomStatus status) {
    if (!db || !room_id) return 0;
    if (rdb_search_room(db, room_id) != NULL) return 0;
    
    ClassroomNode* node = (ClassroomNode*)tracked_malloc(sizeof(ClassroomNode));
    strncpy(node->data.room_id, room_id, 15);
    node->data.room_id[15] = '\0';
    node->data.capacity = capacity;
    node->data.has_projector = has_projector;
    strncpy(node->data.location, location, 63);
    node->data.location[63] = '\0';
    node->data.status = status;
    node->next = NULL;
    node->prev = db->list.tail;
    
    if (db->list.tail) db->list.tail->next = node;
    else db->list.head = node;
    db->list.tail = node;
    db->list.size++;
    
    unsigned int idx = hash_string(room_id) % db->index.bucket_count;
    HashEntry* entry = (HashEntry*)tracked_malloc(sizeof(HashEntry));
    strncpy(entry->room_id, room_id, 15);
    entry->room_id[15] = '\0';
    entry->node = node;
    entry->next = db->index.buckets[idx];
    db->index.buckets[idx] = entry;
    db->index.size++;
    
    return 1;
}

Classroom* rdb_search_room(ResourceDatabase* db, const char* room_id) {
    if (!db || !room_id) return NULL;
    unsigned int idx = hash_string(room_id) % db->index.bucket_count;
    HashEntry* entry = db->index.buckets[idx];
    while (entry) {
        if (strcmp(entry->room_id, room_id) == 0) return &entry->node->data;
        entry = entry->next;
    }
    return NULL;
}

int rdb_remove_room(ResourceDatabase* db, const char* room_id) {
    if (!db || !room_id) return 0;
    unsigned int idx = hash_string(room_id) % db->index.bucket_count;
    HashEntry* entry = db->index.buckets[idx];
    HashEntry* prev_entry = NULL;
    
    while (entry) {
        if (strcmp(entry->room_id, room_id) == 0) {
            if (prev_entry) prev_entry->next = entry->next;
            else db->index.buckets[idx] = entry->next;
            db->index.size--;
            
            ClassroomNode* node = entry->node;
            if (node->prev) node->prev->next = node->next;
            else db->list.head = node->next;
            
            if (node->next) node->next->prev = node->prev;
            else db->list.tail = node->prev;
            db->list.size--;
            
            tracked_free(node);
            tracked_free(entry);
            return 1;
        }
        prev_entry = entry;
        entry = entry->next;
    }
    return 0;
}

void rdb_destroy(ResourceDatabase* db) {
    if (!db) return;
    for (int i = 0; i < db->index.bucket_count; i++) {
        HashEntry* entry = db->index.buckets[i];
        while (entry) {
            HashEntry* next = entry->next;
            tracked_free(entry);
            entry = next;
        }
    }
    tracked_free(db->index.buckets);
    
    ClassroomNode* node = db->list.head;
    while (node) {
        ClassroomNode* next = node->next;
        tracked_free(node);
        node = next;
    }
    tracked_free(db);
}
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* Helper to convert string to lowercase for safe comparisons */
static void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

int rdb_load_from_csv(ResourceDatabase* db, const char* filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Could not open %s\n", filename);
        return -1;
    }

    char line[256];
    int count = 0;

    /* Skip header row */
    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        char room_id[16] = {0};
        int capacity = 0;
        char proj_str[16] = {0};
        char location[64] = {0};
        char status_str[16] = {0};

        /* Parse CSV line separated by commas */
        char *token = strtok(line, ",\n\r");
        if (!token) continue;
        strncpy(room_id, token, 15);

        token = strtok(NULL, ",\n\r");
        if (!token) continue;
        capacity = atoi(token);

        token = strtok(NULL, ",\n\r");
        if (!token) continue;
        strncpy(proj_str, token, 15);

        token = strtok(NULL, ",\n\r");
        if (!token) continue;
        strncpy(location, token, 63);

        token = strtok(NULL, ",\n\r");
        if (!token) continue;
        strncpy(status_str, token, 15);

        /* Map projector boolean */
        to_lowercase(proj_str);
        int has_projector = (strcmp(proj_str, "true") == 0 || strcmp(proj_str, "yes") == 0 || strcmp(proj_str, "1") == 0) ? 1 : 0;

        /* Map status */
        to_lowercase(status_str);
        RoomStatus status = ROOM_AVAILABLE;
        if (strcmp(status_str, "occupied") == 0) status = ROOM_OCCUPIED;
        else if (strcmp(status_str, "maintenance") == 0) status = ROOM_MAINTENANCE;

        /* Add to the Resource Database */
        if (rdb_add_room(db, room_id, capacity, has_projector, location, status)) {
            count++;
        }
    }
    
    fclose(fp);
    return count;
}