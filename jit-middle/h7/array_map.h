#ifndef H7_ARRAY_MAP_H
#define H7_ARRAY_MAP_H

#include "h7/common/dtypes.h"

typedef struct array_map array_map;
typedef array_map* array_map_p;

struct array_map{
    IObject baseObj;

    uint32 capacity;
    uint32 len_entry;
    uint16 key_dt;
    uint16 val_dt;
    float factor;//load factor

    void* keys;
    void* values;
    uint32* hashes;
};
DEF_IOBJ_CHILD_FUNCS(array_map)

array_map_p array_map_new(uint16 key_dt,
                          uint16 val_dt, uint32 init_len);

void array_map_ensure_capacity(array_map_p list, uint32 cap);
void array_map_ensure_size(array_map_p list, uint32 size);
//
void array_map_put(array_map_p ptr, const void* key,
                   const void* value, void* oldVal);
//return 1 if success.
int array_map_get(array_map_p ptr, const void* key, void* oldVal);

void* array_map_rawget(array_map_p ptr, const void* key);

int array_map_remove(array_map_p ptr, const void* key, void* oldVal);

#define array_map_new2(kt,vt) array_map_new(kt, vt, 8)
#define array_map_size(ptr) ptr->len_entry
#define array_map_keys(ptr) ptr->keys
#define array_map_values(ptr) ptr->values

#define array_map_foreach(ptr, code) \
    for(uint32 i = 0 ; i < ptr->len_entry ; ++i){\
        void* key = ((char*)ptr->keys + dt_size(ptr->key_dt) * i);\
        void* value = ((char*)ptr->values + dt_size(ptr->val_dt) * i);\
        code;\
    }

#endif
