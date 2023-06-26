#include "array_map.h"
#include "h7_common.h"
#include <memory.h>
#include "hash.h"
#include "binary_search.h"
#include "mem.h"
#include "h7/h_atomic.h"
#include "h7/common/halloc.h"
#include "h7/h_string.h"

static inline void __array_map_init(array_map_p arr);
static void array_map_prepare_size(array_map_p ptr, uint32 size);

static inline void __grow(array_map_p list, int minCapacity){
   int oldCapacity = list->capacity;
   int newCapacity = oldCapacity + (oldCapacity >> 1);
   if (newCapacity - minCapacity < 0){
       newCapacity = minCapacity;
   }
   array_map_prepare_size(list, newCapacity);
}
void array_map_ensure_capacity(array_map_p list, int cap){
    if((int)list->capacity < cap){
        __grow(list, cap);
    }
}
void array_map_ensure_size(array_map_p list, int size){
    array_map_ensure_capacity(list, size / list->factor + 1);
    list->len_entry = size;
}

static inline void _array_map_copy(int dt, void* srcData,
                                        void* dstData,
                                        int val_len){
    if(val_len == 0){
        return;
    }
    if(dt == kType_P_FUNC){
        memcpy(dstData, srcData, dt_size(dt) * val_len);
    }if(dt_is_pointer(dt)){
        IObject* iobj = ((void**)srcData)[0];
        for(int i = 0 ; i < val_len ; ++i){
            iobj->Func_copy(((void**)srcData)[i], ((void**)dstData)[i]);
        }
    }else{
        memcpy(dstData, srcData, dt_size(dt) * val_len);
    }
}
static inline void _array_map_free(int dt, void* srcData,
                                        int val_len){
    if(val_len == 0){
        return;
    }
    if(dt == kType_P_FUNC){
        FREE(srcData);
    }if(dt_is_pointer(dt)){
        IObject* iobj = ((void**)srcData)[0];
        for(int i = 0 ; i < val_len ; ++i){
            iobj->Func_ref(((void**)srcData)[i], -1);
        }
    }else{
        FREE(srcData);
    }
}
static inline uint32 _array_map_hash(int dt, void* data,
                                   int val_len, uint32 seed){
    if(val_len == 0){
        return seed;
    }
    if(dt == kType_P_FUNC){
        return fasthash32(data, dt_size(dt) * val_len, seed);
    }if(dt_is_pointer(dt)){
        void** sd = (void**)data;
        IObject* iobj = (sd)[0];
        for(int i = 0 ; i < val_len ; ++i){
            seed = iobj->Func_hash(sd[i], seed);
        }
        return seed;
    }else{
        return fasthash32(data, dt_size(dt) * val_len, seed);
    }
}

#define __dump_impl(hffi_t, type, format)\
case hffi_t:{\
    hstring_appendf(hs, format, *((type*)data));\
}break;

static inline void _dump_target(int dt, void* data, hstring* hs){

    if(dt == kType_P_FUNC){
        hstring_appendf(hs, "func<%p>", data);
    }if(dt_is_pointer(dt)){
        IObject* iobj = (IObject*)data;
        iobj->Func_dump(data, hs);
    }else{
        DEF_DT_BASE_SWITCH_FORMAT(__dump_impl, dt);
    }
}
static inline uint32 _array_map_eq(int dt, void* data1,
                                   void* data2,
                                   int val_len){
    if(val_len == 0){
        return kState_OK;
    }
    if(dt == kType_P_FUNC){
        if(memcmp(data1, data2, dt_size(dt) * val_len) != 0){
            return kState_FAILED;
        }
    }if(dt_is_pointer(dt)){
        void** sd1 = (void**)data1;
        void** sd2 = (void**)data2;
        IObject* iobj = (sd1)[0];
        for(int i = 0 ; i < val_len ; ++i){
            if(!iobj->Func_equals(sd1[i], sd2[i])){
                return kState_FAILED;
            }
        }
    }else{
        if(memcmp(data1, data2, dt_size(dt) * val_len) != 0){
            return kState_FAILED;
        }
    }
    return kState_OK;
}
static inline uint32 __hash(int dt, const void* data, int size){
    if(dt == kType_P_FUNC){
        return fasthash32(data, size, DEFAULT_HASH_SEED);
    }if(dt_is_pointer(dt)){
        IObject* iobj = (IObject*)data;
        return iobj->Func_hash((void*)data, DEFAULT_HASH_SEED);
    }else{
        return fasthash32(data, size, DEFAULT_HASH_SEED);
    }
}
//-------------------------------------------
IObjPtr (Func_copy0)(IObjPtr src1, IObjPtr dst1){
    array_map_p ptr1 = (array_map_p)src1;
    array_map_p dst;
    if(dst1){
        dst = (array_map_p)dst1;
    }else{
        dst = array_map_new(ptr1->key_dt, ptr1->val_dt,
                            ptr1->capacity);
        __array_map_init(dst);
    }
    uint32 init_len = ptr1->capacity;
    uint16 key_unit_size = dt_size(ptr1->key_dt);
    uint16 val_unit_size = dt_size(ptr1->val_dt);
    uint32 val_len = ptr1->len_entry;
    //
    array_map_p ptr = dst;
    ptr->keys = ALLOC(init_len * key_unit_size);
    ptr->values = ALLOC(init_len * val_unit_size);
    ptr->hashes = ALLOC(init_len * sizeof (uint32));
    ptr->capacity = init_len;
    ptr->len_entry = ptr1->len_entry;

    _array_map_copy(ptr1->key_dt, ptr1->keys, ptr->keys, val_len);
    _array_map_copy(ptr1->val_dt, ptr1->values, ptr->values, val_len);
    memcpy(ptr->hashes, ptr1->hashes, init_len * sizeof (uint32));
    return ptr;
}
int (Func_equals0)(IObjPtr src1, IObjPtr dst1){
    array_map_p src = (array_map_p)src1;
    array_map_p dst = (array_map_p)dst1;
    if(src->key_dt != dst->key_dt || src->val_dt != dst->val_dt){
        return kState_FAILED;
    }
    if(src->len_entry != dst->len_entry){
        return kState_FAILED;
    }
    if(memcmp(src->hashes, dst->hashes,
              sizeof(uint32) * src->len_entry) != 0){
        return kState_FAILED;
    }
    if(!_array_map_eq(src->key_dt, src->keys,
                     dst->keys, src->len_entry)){
        return kState_FAILED;
    }
    if(!_array_map_eq(src->val_dt, src->values,
                     dst->values, src->len_entry)){
        return kState_FAILED;
    }
    return kState_OK;
}
uint32 (Func_hash0)(IObjPtr src, uint32 hash){
    array_map_p ptr1 = (array_map_p)src;
    uint32 nhash = _array_map_hash(ptr1->key_dt, ptr1->keys,
                    ptr1->len_entry, hash);
    nhash = _array_map_hash(ptr1->val_dt, ptr1->values,
                        ptr1->len_entry, nhash);
    return nhash;
}
void (Func_dump0)(IObjPtr src, hstring* hs){
     array_map_p ptr1 = (array_map_p)src;
    //_dump_target
    hstring_append(hs, "{");
    for(uint32 i = 0 ; i < ptr1->len_entry ; ++i){
        hstring_append(hs, "(");
        _dump_target(ptr1->key_dt, ptr1->keys, hs);
        hstring_append(hs, ":");
        _dump_target(ptr1->val_dt, ptr1->values, hs);
        hstring_append(hs, ")");
        if(i != ptr1->len_entry - 1){
            hstring_append(hs, ", ");
        }
    }
    hstring_append(hs, "}");
}
void (Func_ref0)(IObjPtr src, int c){
    array_map_p ptr = (array_map_p)src;
    if(h_atomic_add(&ptr->baseObj.ref, c) == -c){
        _array_map_free(ptr->key_dt, ptr->keys, ptr->len_entry);
        _array_map_free(ptr->val_dt, ptr->values, ptr->len_entry);
        FREE(ptr->hashes);
        FREE(ptr);
    }
}

static inline void __array_map_init(array_map_p arr){
    IObject_set_name(arr, "array_map");
    arr->baseObj.ref = 1;
    arr->baseObj.Func_copy = Func_copy0;
    arr->baseObj.Func_dump = Func_dump0;
    arr->baseObj.Func_equals = Func_equals0;
    arr->baseObj.Func_hash = Func_hash0;
    arr->baseObj.Func_ref = Func_ref0;
}

//--------------------------------------------------
array_map_p array_map_new(uint16 key_dt,
                          uint16 val_dt, uint32 init_len){
    array_map_p ptr = ALLOC(sizeof (struct array_map));
    __array_map_init(ptr);
    ptr->keys = ALLOC(init_len * dt_size(key_dt));
    ptr->values = ALLOC(init_len * dt_size(val_dt));
    ptr->hashes = ALLOC(init_len * sizeof (uint32));
    ptr->capacity = init_len;
    ptr->val_dt = val_dt;
    ptr->key_dt = key_dt;
    ptr->len_entry = 0;
    ptr->factor = 0.75f;
    return ptr;
}

void array_map_put(array_map_p ptr, const void* key,
                   const void* value, void* oldVal){

    uint32 key_ele_size = dt_size(ptr->key_dt);
    uint32 val_ele_size = dt_size(ptr->val_dt);
    //handle hash.
    uint32 hash = __hash(ptr->key_dt, key, key_ele_size);
    if(ptr->len_entry == 0){
        arrays_insert(ptr->keys, 0, key_ele_size, key, 0);
        arrays_insert(ptr->values, 0, val_ele_size, value, 0);
        arrays_insert(ptr->hashes, 0, sizeof (uint32), &hash, 0);
        return;
    }
    int pos = binarySearch_uint32(ptr->hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        // void* key_dst = (char*)ptr->keys + ptr->key_ele_size * ptr->len_entry;
        void* val_dst = (char*)ptr->values + val_ele_size * ptr->len_entry;
        //exist
        if(oldVal){
            memcpy(oldVal, val_dst, val_ele_size);
        }
        memcpy(val_dst, value, val_ele_size);
    }else{
        // -(pos + 1)
        int pos1 = -(pos + 1);
        arrays_insert(ptr->keys, ptr->len_entry, key_ele_size, key, pos1);
        arrays_insert(ptr->values, ptr->len_entry, val_ele_size, value, pos1);
        arrays_insert(ptr->hashes, ptr->len_entry,
                      sizeof (uint32), &hash, pos1);
        ptr->len_entry ++;
    }
    //add if need
    if(ptr->len_entry >= ptr->factor * ptr->capacity){
        __grow(ptr, 0);
    }
}

void array_map_prepare_size(array_map_p ptr, uint32 size){
    ASSERT(ptr->capacity < size);
    uint32 key_ele_size = dt_size(ptr->key_dt);
    uint32 val_ele_size = dt_size(ptr->val_dt);
    ptr->keys = REALLOC(ptr->keys, ptr->capacity * key_ele_size,
                                 size * key_ele_size);
    ptr->values = REALLOC(ptr->values, ptr->capacity * val_ele_size,
                                 size * val_ele_size);
    ptr->hashes = REALLOC(ptr->hashes, ptr->capacity * sizeof (uint32),
                                 size * sizeof (uint32));
    ptr->capacity = size;
}

int array_map_get(array_map_p ptr, const void* key, void* oldVal){
    ASSERT(oldVal != NULL);
    ASSERT(key != NULL);
    uint32 key_ele_size = dt_size(ptr->key_dt);
    uint32 val_ele_size = dt_size(ptr->val_dt);
    //handle hash.
    uint32 hash = __hash(ptr->key_dt, key, key_ele_size);
    int pos = binarySearch_uint32(ptr->hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        void* val_dst = (char*)ptr->values + val_ele_size * pos;
        memcpy(oldVal, val_dst, val_ele_size);
        return 1;
    }
    return 0;
}

void* array_map_rawget(array_map_p ptr, const void* key){
    ASSERT(key != NULL);
    uint32 key_ele_size = dt_size(ptr->key_dt);
    uint32 val_ele_size = dt_size(ptr->val_dt);
    //handle hash.
    uint32 hash = __hash(ptr->key_dt, key, key_ele_size);
    int pos = binarySearch_uint32(ptr->hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        //void* val_dst = (char*)ptr->values + ptr->val_ele_size * ptr->len_entry;
        return (char*)ptr->values + val_ele_size * pos;
    }
    return NULL;
}

int array_map_remove(array_map_p ptr, const void* key, void* oldVal){
    ASSERT(key != NULL);
    uint32 key_ele_size = dt_size(ptr->key_dt);
    uint32 val_ele_size = dt_size(ptr->val_dt);
    //handle hash.
    uint32 hash = __hash(ptr->key_dt, key, key_ele_size);
    int pos = binarySearch_uint32(ptr->hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        if(oldVal){
            void* val_dst = (char*)ptr->values + val_ele_size * ptr->len_entry;
            memcpy(oldVal, val_dst, val_ele_size);
        }
        arrays_remove(ptr->keys, ptr->len_entry, key_ele_size, pos);
        arrays_remove(ptr->values, ptr->len_entry, val_ele_size, pos);
        arrays_remove(ptr->hashes, ptr->len_entry, sizeof (uint32), pos);
        ptr->len_entry -- ;
        return 1;
    }
    return 0;
}


