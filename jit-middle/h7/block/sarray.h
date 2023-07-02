#ifndef h_SARRAY_H_
#define h_SARRAY_H_

#include <stdlib.h>

#define SARRAY_DC 8

#define SARRAY_INIT(st)\
typedef struct sa_##st{\
    int len;\
    int cap;\
    void** arr;\
}sa_##st, *sa_##st##_p;\
static inline sa_##st * sa_##st##_new(){\
    sa_##st* p = (sa_##st*)malloc(sizeof(sa_##st));\
    p->len = 0;\
    p->cap = SARRAY_DC;\
    p->arr = (void**)malloc(sizeof(void*) * (size_t)p->cap);\
    return p;\
}\
static inline void sa_##st##_add(sa_##st* p, void* st){\
    p->arr[p->len++] = st;\
    if(p->len == p->cap -1){\
        p->arr = (void**)realloc((void*)p->arr, sizeof(void*) *(size_t)(p->cap + SARRAY_DC));\
    }\
}\
static inline void sa_##st##_delete(sa_##st* p){\
    for(int i = 0 ; i < p-> len ; ++i){\
        free(p->arr[i]);\
    }\
    free(p->arr);\
    free(p);\
}\
static inline void sa_##st##_delete_func(sa_##st* p, void(*func)(void*)){\
    for(int i = 0 ; i < p-> len ; ++i){\
        func(p->arr[i]);\
    }\
    free(p->arr);\
    free(p);\
}\

#endif
