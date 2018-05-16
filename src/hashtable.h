#ifndef _hashtable_h
#define _hashtable_h

typedef struct hashentry_t hashentry_t;

typedef struct {
    size_t num_slots;
    size_t count;
    hashentry_t** slots;
    // keep object pool of hash entries!
} hashtable_t;

hashtable_t* ht_new(size_t capacity);
void ht_free(hashtable_t** ht);

void ht_put(hashtable_t* ht, char* key, void* value);
// if key in ht => return 1, write data in value, else: return 0
int ht_get(hashtable_t* ht, char* key, void** value);
void ht_remove(hashtable_t* ht, char* key);

#endif
