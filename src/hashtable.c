#include <stdlib.h>
#include <stdio.h>

#include "hashtable.h"

typedef unsigned long hashentry_key_t;

struct hashentry_t {
    hashentry_key_t key;
    void* data;
    struct hashentry_t* next;
};

static hashentry_t* he_new(hashentry_key_t key, void* data, hashentry_t* next)
{
    hashentry_t* he = malloc(sizeof(hashentry_t));
    if (!he)
        return NULL;
    he->key = key;
    he->data = data;
    he->next = next;
    return he;
}

static void he_free(hashentry_t** he)
{
    if (!*he)
        return;
    // remove recursion -> outside code needs to iterate?
    he_free(&(*he)->next);
    free((*he)->data);
    free(*he);
    *he = NULL;
}

static hashentry_key_t djb2_hash(char* str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

hashtable_t* ht_new(size_t capacity)
{
    hashtable_t* ht = malloc(sizeof(hashtable_t));
    if (!ht)
        return NULL;

    ht->capacity = capacity;
    ht->count = 0;
    ht->entries = malloc(sizeof(hashentry_t*) * capacity);
    if (!ht->entries)
    {
        free(ht);
        return NULL;
    }

    return ht;
}

void ht_free(hashtable_t** ht)
{
    hashtable_t* h = *ht;
    if (!h)
        return;
    for (size_t i = 0; i < h->capacity; ++i)
    {
        he_free(&(h->entries[i]));
    }
    free(h);
    *ht = NULL;
}
