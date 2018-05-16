#include <stdlib.h>
#include <stdio.h>

#include "hashtable.h"

typedef unsigned long hashentry_key_t;

struct hashentry_t {
    hashentry_key_t key; // 4 byte
    // frees data ptr if != 0
    char free_data; // 1 byte
    char _pad[3]; // 3 byte
    void* data; // 8 byte
    struct hashentry_t* next; // 8 byte
};

static hashentry_t* he_new(hashentry_key_t key, char free_data, void* data, hashentry_t* next)
{
    hashentry_t* he = malloc(sizeof(hashentry_t));
    if (!he)
        return NULL;
    he->key = key;
    he->free_data = free_data;
    he->data = data;
    he->next = next;
    return he;
}

static void he_free(hashentry_t* he)
{
    if (!he)
        return;
    if (he->free_data && he->data)
        free(he->data);
    free(he);
}

static hashentry_key_t hash_djb2(char* str)
{
    hashentry_key_t hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

hashtable_t* ht_new(size_t num_slots)
{
    hashentry_t t;
    printf("entry: %i\n", sizeof(t));
    printf("  key: %i\n", sizeof(t.key));
    printf("  free_data: %i\n", sizeof(t.free_data));
    printf("  _pad: %i\n", sizeof(t._pad));
    printf("  data: %i\n", sizeof(t.data));
    printf("  next: %i\n\n", sizeof(t.next));

    printf("table: %i\n", sizeof(hashtable_t));
    printf("  capacity: %i\n", sizeof(size_t));
    printf("  count: %i\n", sizeof(size_t));
    printf("  slots: %i\n\n", sizeof(hashentry_t**));

    hashtable_t* ht = malloc(sizeof(hashtable_t));
    if (!ht)
        return NULL;

    ht->num_slots = num_slots;
    ht->count = 0;
    ht->slots = calloc(num_slots, sizeof(hashentry_t*));
    if (!ht->slots)
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
    for (size_t i = 0; i < h->num_slots; ++i)
    {
        hashentry_t* entry = h->slots[i];
        while (entry)
        {
            hashentry_t* next = entry->next;

            he_free(entry);

            entry = next;
        }
    }
    free(h->slots);
    free(h);
    *ht = NULL;
}

void ht_put(hashtable_t* ht, char* key, void* value, char free_data)
{
    if (!ht)
        return;
    hashentry_key_t hash = hash_djb2(key);

    // calc index
    size_t idx = hash % ht->num_slots;
    // get linked list of entries
    hashentry_t* first = ht->slots[idx];
    
    if (first)
    {
        // collision!
        // see if key is already in hashtable
        hashentry_t* node = first;
        while (node)
        {
            if (node->key == hash)
            {
                // replace data
                if (node->free_data && node->data)
                    free(node->data);

                node->data = value;
                node->free_data = free_data;
                return;
            }
            node = node->next;
        }
        // no entry with same key found
        // insert at beginning of list
        ht->slots[idx] = he_new(hash, free_data, value, ht->slots[idx]);
        ht->count++;
    }
    else
    {
        // no entry with this key yet
        ht->slots[idx] = he_new(hash, free_data, value, NULL);
        ht->count++;
    }
}

// if key in ht => return 1, write data in value, else: return 0
int ht_get(hashtable_t* ht, char* key, void** value)
{
    if (!ht)
        return 0;
    hashentry_key_t hash = hash_djb2(key);

    size_t idx = hash % ht->num_slots;
    hashentry_t* first = ht->slots[idx];

    if (first)
    {
        hashentry_t* entry = first;
        while (entry)
        {
            if (entry->key == hash)
            {
                *value = entry->data;
                return 1;
            }
            entry = entry->next;
        }
    }
    return 0;
}

void ht_remove(hashtable_t* ht, char* key)
{
    if (!ht)
        return;
    hashentry_key_t hash = hash_djb2(key);

    size_t idx = hash % ht->num_slots;
    hashentry_t* first = ht->slots[idx];

    if (first)
    {
        if (first->key == hash)
        {
            ht->slots[idx] = first->next;

            he_free(first);
            return;
        }

        hashentry_t* entry = first->next;

        while (entry)
        {
            if (entry->key == hash)
            {
                first->next = entry->next;
                he_free(entry);
                return;
            }

            first = entry;
            entry = entry->next;
        }
    }
}
