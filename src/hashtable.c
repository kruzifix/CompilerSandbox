#include <stdlib.h>
#include <stdio.h>

#include "hashtable.h"

static hashentry_key_t djb2_hash(char* str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}


