﻿#include "array_map.h"
#include "h7_common.h"
#include <memory.h>
#include "hash.h"
#include "binary_search.h"
#include "mem.h"
#include "atomic.h"

#define HASH_SEED 11

static inline void growUpIfNeed(array_map_p ptr, uint32 delta){
    if(ptr->len_entry >= ptr->capacity - delta){
        int c1 = ptr->capacity << 1;
        int c2 = ptr->len_entry + delta;
        array_map_prepare_size(ptr, HMAX(c1, c2));
    }
}

array_map_p array_map_new(struct core_allocator* ca, uint16 key_unit_size,
                          uint16 val_unit_size, uint32 init_len){
    array_map_p ptr = (array_map_p)ca->Alloc(sizeof (struct array_map));
    ptr->ca = ca;
    ptr->ref = 1;
    ptr->keys = ca->Alloc(init_len * key_unit_size);
    ptr->values = ca->Alloc(init_len * val_unit_size);
    ptr->hashes = ca->Alloc(init_len * sizeof (uint32));
    ptr->capacity = init_len;
    ptr->len_entry = 0;
    ptr->key_ele_size = key_unit_size;
    ptr->val_ele_size = val_unit_size;
    ptr->cpy_key = NULL;
    ptr->cpy_value = NULL;
    return ptr;
}

void array_map_put(array_map_p ptr, const void* key,
                   const void* value, void* oldVal){

    //handle hash.
    uint32 hash = fasthash32(key, ptr->key_ele_size, HASH_SEED);
    if(ptr->len_entry == 0){
        growUpIfNeed(ptr, 1);
        arrays_insert(ptr->keys, 0, ptr->key_ele_size, key, 0);
        arrays_insert(ptr->values, 0, ptr->val_ele_size, value, 0);
        arrays_insert(ptr->hashes, 0, sizeof (uint32), &hash, 0);
        return;
    }
    int pos = binarySearch_uint32(ptr->hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        // void* key_dst = (char*)ptr->keys + ptr->key_ele_size * ptr->len_entry;
        void* val_dst = (char*)ptr->values + ptr->val_ele_size * ptr->len_entry;
        //exist
        if(oldVal){
            memcpy(oldVal, val_dst, ptr->val_ele_size);
        }
        memcpy(val_dst, value, ptr->val_ele_size);
    }else{
        growUpIfNeed(ptr, 1);
        // -(pos + 1)
        int pos1 = -(pos + 1);
        arrays_insert(ptr->keys, ptr->len_entry, ptr->key_ele_size, key, pos1);
        arrays_insert(ptr->values, ptr->len_entry, ptr->val_ele_size, value, pos1);
        arrays_insert(ptr->hashes, ptr->len_entry, sizeof (uint32), &hash, pos1);
        ptr->len_entry ++;
    }
}

void array_map_prepare_size(array_map_p ptr, uint32 size){
    ASSERT(ptr->capacity < size);
    ptr->keys = ptr->ca->Realloc(ptr->keys, ptr->capacity * ptr->key_ele_size,
                                 size * ptr->key_ele_size);
    ptr->values = ptr->ca->Realloc(ptr->values, ptr->capacity * ptr->val_ele_size,
                                 size * ptr->val_ele_size);
    ptr->hashes = ptr->ca->Realloc(ptr->hashes, ptr->capacity * sizeof (uint32),
                                 size * sizeof (uint32));
    ptr->capacity = size;
}

int array_map_get(array_map_p ptr, const void* key, void* oldVal){
    ASSERT(oldVal != NULL);
    ASSERT(key != NULL);
    //handle hash.
    uint32 hash = fasthash32(key, ptr->key_ele_size, HASH_SEED);
    int pos = binarySearch_uint32(ptr->hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        void* val_dst = (char*)ptr->values + ptr->val_ele_size * pos;
        memcpy(oldVal, val_dst, ptr->val_ele_size);
        return 1;
    }
    return 0;
}

void* array_map_rawget(array_map_p ptr, const void* key){
    ASSERT(key != NULL);
    //handle hash.
    uint32 hash = fasthash32(key, ptr->key_ele_size, HASH_SEED);
    int pos = binarySearch_uint32(ptr->hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        //void* val_dst = (char*)ptr->values + ptr->val_ele_size * ptr->len_entry;
        return (char*)ptr->values + ptr->val_ele_size * pos;
    }
    return NULL;
}

int array_map_remove(array_map_p ptr, const void* key, void* oldVal){
    ASSERT(key != NULL);
    //handle hash.
    uint32 hash = fasthash32(key, ptr->key_ele_size, HASH_SEED);
    int pos = binarySearch_uint32(ptr->hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        if(oldVal){
            void* val_dst = (char*)ptr->values + ptr->val_ele_size * ptr->len_entry;
            memcpy(oldVal, val_dst, ptr->val_ele_size);
        }
        arrays_remove(ptr->keys, ptr->len_entry, ptr->key_ele_size, pos);
        arrays_remove(ptr->values, ptr->len_entry, ptr->val_ele_size, pos);
        arrays_remove(ptr->hashes, ptr->len_entry, sizeof (uint32), pos);
        ptr->len_entry -- ;
        return 1;
    }
    return 0;
}

array_map_p array_map_copy(array_map_p ptr1){
    return array_map_deep_copy(ptr1, NULL, NULL);
}

array_map_p array_map_deep_copy(array_map_p ptr1, Func_Map_KV_cpy fk,
                                Func_Map_KV_cpy fv){
    struct core_allocator* ca = ptr1->ca;
    uint32 init_len = ptr1->capacity;
    uint16 key_unit_size = ptr1->key_ele_size;
    uint16 val_unit_size = ptr1->val_ele_size;
    uint32 val_len = ptr1->len_entry;
    //
    array_map_p ptr = (array_map_p)ca->Alloc(sizeof (struct array_map));
    ptr->ref = 1;
    ptr->ca = ca;
    ptr->keys = ca->Alloc(init_len * key_unit_size);
    ptr->values = ca->Alloc(init_len * val_unit_size);
    ptr->hashes = ca->Alloc(init_len * sizeof (uint32));
    ptr->capacity = init_len;
    ptr->len_entry = ptr1->len_entry;
    ptr->key_ele_size = key_unit_size;
    ptr->val_ele_size = val_unit_size;

    if(fk){
        for(uint32 i = 0 ; i < val_len; ++ i){
            fk(((void**)ptr1->keys)[i], ((void**)ptr->keys)[i]);
        }
    }else{
        memcpy(ptr->keys, ptr1->keys, val_len * key_unit_size);
    }
    if(fv){
        for(uint32 i = 0 ; i < val_len; ++ i){
            fv(((void**)ptr1->values)[i], ((void**)ptr->values)[i]);
        }
    }else{
        memcpy(ptr->values, ptr1->values, val_len * val_unit_size);
    }
    memcpy(ptr->hashes, ptr1->hashes, init_len * sizeof (uint32));
    return ptr;
}

void array_map_delete(array_map_p ptr){
    if(atomic_add(&ptr->ref, -1) == 1){
        struct core_allocator* ca = ptr->ca;
        ca->Free(ptr->keys);
        ca->Free(ptr->values);
        ca->Free(ptr->hashes);
        ca->Free(ptr);
    }
}

