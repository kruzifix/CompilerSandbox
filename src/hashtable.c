#include <stdlib.h>
#include <stdio.h>

#include "hashtable.h"

typedef unsigned long hashentry_key_t;

// hashentry holds data as void*
// this memory gets freed, when the hashentry is freed
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

static hashentry_key_t hash_djb2(char* str)
{
    hashentry_key_t hash = 5381;
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
    ht->entries = calloc(capacity, sizeof(hashentry_t*));
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
        hashentry_t* entry = h->entries[i];
        while (entry)
        {
            hashentry_t* next = entry->next;

            if (entry->data)
                free(entry->data);
            free(entry);

            entry = next;
        }
    }
    free(h->entries);
    free(h);
    *ht = NULL;
}

void ht_put(hashtable_t* ht, char* key, void* value)
{
    if (!ht)
        return;
    hashentry_key_t hash = hash_djb2(key);

    // calc index
    size_t idx = hash % ht->capacity;
    // get linked list of entries
    hashentry_t* first = ht->entries[idx];

    printf("adding entry with key %s  ->  %i\nmapping to slot %i\n", key, hash, idx);

    if (first == NULL)
    {
        // no entry with this key yet
        ht->entries[idx] = he_new(hash, value, NULL);
        ht->count++;
    }
    else
    {
        // collision!
        // see if key is already in hashtable
        hashentry_t* node = first;
        while (node)
        {
            if (node->key == hash)
            {
                // replace data
                printf("collision! replace data!\n");
                node->data = value;
                return;
            }
            node = node->next;
        }
        // no entry with same key found
        // insert at beginning of list
        printf("collision! insert new entry!\n");
        ht->entries[idx] = he_new(hash, value, ht->entries[idx]);
    }
}

void* ht_get(hashtable_t* ht, char* key)
{
    return NULL;
}
