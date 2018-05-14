#ifndef _stack_allocator_h
#define _stack_allocator_h

typedef struct {
    size_t size;
    size_t free;
    size_t num_allocs;
    char* stack;
    char* top;
} stack_allocator_t;

stack_allocator_t* sa_new(size_t size);
void sa_free(stack_allocator_t** sa);

void sa_reset(stack_allocator_t* sa);
void* sa_alloc(stack_allocator_t* sa, size_t size);

void sa_print_stats(stack_allocator_t* sa);

#endif
