#ifndef _hashtable_h
#define _hashtable_h

typedef struct hashentry_t hashentry_t;

typedef struct {
    size_t capacity;
    size_t count;
    hashentry_t** entries;
} hashtable_t;

hashtable_t* ht_new(size_t capacity);
void ht_free(hashtable_t** ht);

void ht_put(char* key, void* value);
void* ht_get(char* key);

#endif