#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <errno.h>
#include "h7/common/halloc.h"

static struct core_allocator* _alloc = NULL;

static void* wrap_realloc(void *ptr, size_t oldSize, size_t newSize){
    void *p = realloc(ptr, newSize);
    if (NULL == p) {
        fprintf(stderr,
                "[%p] Failed to allocate %zu bytes.", ptr, newSize);
        exit(EXIT_FAILURE);
    }
    return p;
}

void h7_set_core_allocator(struct core_allocator* alloc){
    _alloc = alloc;
}
struct core_allocator* h7_get_core_allocator(){
    if(_alloc == NULL){
        _alloc = malloc(sizeof(core_allocator));
        _alloc->Alloc = malloc;
        _alloc->Realloc = wrap_realloc;
        _alloc->Free = free;
    }
    return _alloc;
}
