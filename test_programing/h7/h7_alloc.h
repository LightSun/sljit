#pragma once

#include <stdlib.h>
#include "h7/common/c_common.h"

//later align memory
#define H7_NEW(s) malloc(s)
#define H7_NEW_COUNT_TYPE(t, c) malloc(sizeof(t) * c)
#define H7_DELETE(ptr) free(ptr)
#define H7_NEW_TYPE(T) (T*)malloc(sizeof(T))

#define H7_NEW_OBJ(T) new T()
#define H7_NEW_OBJ1(T, p) new T(p)
#define H7_DELETE_OBJ(p) delete p

#define H7_REALLOC(ptr, oldS, newS) realloc(ptr, newS)

struct _MemoryBlock{
    void* data;
    uint32 size;
    uint32 allocSize;
};
