#ifndef _hashtable_h
#define _hashtable_h

typedef unsigned long hashentry_key_t;

typedef struct {
    hashentry_key_t key;
    void* data;
    struct hashentry_t* next;
} hashentry_t;

typedef struct {
    size_t capacity;
    size_t count;
    hashentry_t* entries;
} hashtable_t;

hashtable_t* ht_new(size_t capacity);
void ht_free(hashtable_t** ht);

void ht_put(char* key, void* value);
void* ht_get(char* key);

#endif
