#ifndef _hashtable_h
#define _hashtable_h

typedef struct hashentry_t hashentry_t;

typedef struct {
    size_t capacity;
    size_t count;
    hashentry_t** entries;
    // keep object pool of hash entries!
} hashtable_t;

hashtable_t* ht_new(size_t capacity);
void ht_free(hashtable_t** ht);

void ht_put(hashtable_t* ht, char* key, void* value);
void* ht_get(hashtable_t* ht, char* key);

#endif
