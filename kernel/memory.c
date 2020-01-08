#ifndef MEMORY_C
#define MEMORY_C

#include <stddef.h>

void* _curr_end = (void*) 0x200000;

void* alloc(size_t size) {
    void* thanks_jp = _curr_end;
    _curr_end += size;
    return thanks_jp;
}
#endif // MEMORY_C