#ifndef _config_h_
#define _config_h_

#include "stack_allocator.h"

extern stack_allocator_t* global_stack_alloc;
#define _MALLOC(s) sa_alloc(global_stack_alloc, (s))

#define EXIT(msg) panic_exit((msg), __FILE__, __LINE__)
void panic_exit(const char* message, char* file, int line);

#endif
