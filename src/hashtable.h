#ifndef _hashtable_h
#define _hashtable_h

#define HASHTABLE_INCLUDE_KEY_IN_ENTRY

typedef unsigned long hashentry_key_t;
typedef struct hashentry_t hashentry_t;

struct hashentry_t {
#ifdef HASHTABLE_INCLUDE_KEY_IN_ENTRY
    char* key_value; // 8 byte
#endif
    hashentry_key_t key; // 4 byte
    // frees data ptr if != 0
    char free_data; // 1 byte
    char _pad[3]; // 3 byte
    void* data; // 8 byte
    hashentry_t* next; // 8 byte
};

typedef struct {
    size_t num_slots; // 8 byte
    size_t count; // 8 byte
    hashentry_t** slots; // 8 byte
    // keep object pool of hash entries!
} hashtable_t;

hashtable_t* ht_new(size_t capacity);
void ht_free(hashtable_t** ht);

void ht_clear(hashtable_t* ht);
void ht_put(hashtable_t* ht, char* key, void* value, char free_data);
// if key in ht => return 1, write data in value; else: return 0
int ht_get(hashtable_t* ht, char* key, void** value);
void ht_remove(hashtable_t* ht, char* key);

void ht_repl(hashtable_t* ht);

#endif
