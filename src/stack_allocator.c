#include <stdlib.h>
#include <stdio.h>

#include "stack_allocator.h"

stack_allocator_t* sa_new(size_t size)
{
    stack_allocator_t* sa = malloc(sizeof(stack_allocator_t));
    if (!sa)
        return NULL;
    sa->size = size;
    sa->free = size;
    sa->num_allocs = 0;
    sa->stack = calloc(size, 1);
    if (!sa->stack)
    {
        free(sa);
        return NULL;
    }
    sa->top = sa->stack;
    return sa;
}

void sa_free(stack_allocator_t** sa)
{
    if (!*sa)
        return;
    free((*sa)->stack);
    free(*sa);
    *sa = NULL;
}

void sa_reset(stack_allocator_t* sa)
{
    if (!sa)
        return;
    sa->top = sa->stack;
    sa->free = sa->size;
    sa->num_allocs = 0;
}

void* sa_alloc(stack_allocator_t* sa, size_t size)
{
    if (!sa)
        return NULL;
    // check if enough memory is free
    if (size > sa->free)
    {
        printf("stack allocator: allocating %i bytes failed. only %i free.\n", size, sa->free);
        return NULL;
    }
    sa->free -= size;
    void* ptr = sa->top;
    sa->top += size;
    sa->num_allocs++;
    return ptr;
}

void sa_print_stats(stack_allocator_t* sa)
{
    printf("\n --- STACK STATS ---\n");
    printf("size: %i\n", sa->size);
    printf("free: %i\n", sa->free);
    printf("used: %i\n", sa->size - sa->free);
    printf("allocations: %i\n", sa->num_allocs);
    printf(" -------------------\n");
}
