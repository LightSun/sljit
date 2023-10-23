#include <memory.h>
#include "h7/array_map.h"
#include "h7/h7_common.h"
#include "h7/hash.h"
#include "h7/binary_search.h"
#include "h7/mem.h"
#include "h7/h_atomic.h"
#include "h7/common/halloc.h"
#include "h7/h_string.h"
#include "h7/numbers.h"

static inline void __array_map_init(array_map_p arr);
static void array_map_prepare_size(array_map_p ptr, uint32 size);

static inline void __grow(array_map_p list, int minCapacity){
   int oldCapacity = (int)list->capacity;
   int newCapacity = oldCapacity + (oldCapacity >> 1);
   if (newCapacity - minCapacity < 0){
       newCapacity = minCapacity;
   }
   array_map_prepare_size(list, (uint32)newCapacity);
}
void array_map_ensure_capacity(array_map_p list, uint32 cap){
    if(list->capacity < cap){
        __grow(list, (int)cap);
    }
}
void array_map_ensure_size(array_map_p list, uint32 size){
    array_map_ensure_capacity(list, (uint32)((float)size / list->factor) + 1);
}

static inline void _array_map_copy(int dt, void* srcData,
                                        void* dstData,
                                        uint32 val_len){
    if(val_len == 0){
        return;
    }
    if(dt_is_pointer(dt)){
        IObject* iobj = ((void**)srcData)[0];
        for(uint32 i = 0 ; i < val_len ; ++i){
            iobj->class_info->Func_copy(((void**)srcData)[i], ((void**)dstData)[i]);
        }
    }else{
        memcpy(dstData, srcData, dt_size(dt) * val_len);
    }
}
static inline void _array_map_free(int dt, void* srcData,
                                        uint32 val_len){
    if(val_len == 0){
        return;
    }
    if(dt_is_pointer(dt)){
        IObject* iobj = ((void**)srcData)[0];
        for(uint32 i = 0 ; i < val_len ; ++i){
            iobj->class_info->Func_ref(((void**)srcData)[i], -1);
        }
    }else{
        FREE(srcData);
    }
}
static inline uint32 _array_map_hash(int dt, void* data,
                                   uint32 val_len, uint32 seed){
    if(val_len == 0){
        return seed;
    }
    if(dt_is_pointer(dt)){
        void** sd = (void**)data;
        IObject* iobj = (sd)[0];
        for(uint32 i = 0 ; i < val_len ; ++i){
            seed = iobj->class_info->Func_hash(sd[i], seed);
        }
        return seed;
    }else{
        return fasthash32(data, dt_size(dt) * val_len, seed);
    }
}

#define __dump_impl(hffi_t, type, format)\
case hffi_t:{\
    hstring_appendf(hs, format, ((type*)data)[index]);\
}break;

static inline void _dump_target(int dt, void* data, uint32 index, hstring* hs){

    if(dt_is_pointer(dt)){
        IObject* iobj = (IObject*)data;
        iobj->class_info->Func_dump(data, hs);
    }else{
        //DEF_DT_BASE_SWITCH_FORMAT(__dump_impl, dt);
    }
}
static inline uint32 _array_map_eq(int dt, void* data1,
                                   void* data2,
                                   int val_len){
    if(val_len == 0){
        return kState_OK;
    }
    if(dt_is_pointer(dt)){
        void** sd1 = (void**)data1;
        void** sd2 = (void**)data2;
        IObject* iobj = (sd1)[0];
        for(int i = 0 ; i < val_len ; ++i){
            if(!iobj->class_info->Func_equals(sd1[i], sd2[i])){
                return kState_FAILED;
            }
        }
    }else{
        if(dt == kType_F32){
            for(int i = 0 ; i < val_len ; i ++){
                if(!isFloatEquals(((float*)data1)[i],
                                  ((float*)data2)[i])
                        ){
                    return kState_FAILED;
                }
            }
        }else if(dt == kType_F64){
            for(int i = 0 ; i < val_len ; i ++){
                if(!isDoubleEquals(((float*)data1)[i],
                                  ((float*)data2)[i])
                        ){
                    return kState_FAILED;
                }
            }
        }else{
            if (memcmp(data1, data2, dt_size(dt) *(uint32) val_len) != 0){
                return kState_FAILED;
            }
        }
    }
    return kState_OK;
}
static inline uint32 __hash(int dt, const void* data, uint32 size){

    if(dt_is_pointer(dt)){
        IObject* iobj = (IObject*)data;
        return iobj->class_info->Func_hash((void*)data, DEFAULT_HASH_SEED);
    }else{
        return fasthash32(data, size, DEFAULT_HASH_SEED);
    }
}
//-------------------------------------------
static IObjPtr (Func_copy0)(IObjPtr src1, IObjPtr dst1){
    array_map_p ptr1 = (array_map_p)src1;
    array_map_p dst;
    //
    uint32 init_len = ptr1->capacity;
    uint32 val_len = ptr1->len_entry;
    if(dst1){
        dst = (array_map_p)dst1;
        dst->key_dt = ptr1->key_dt;
        dst->val_dt = ptr1->val_dt;
        dst->factor = ptr1->factor;
        array_map_ensure_capacity(dst, ptr1->capacity);
    }else{
        dst = array_map_new(ptr1->key_dt, ptr1->val_dt,
                            ptr1->capacity);
        __array_map_init(dst);
    }
    //
    array_map_p ptr = dst;
    ptr->capacity = init_len;
    ptr->len_entry = ptr1->len_entry;

    _array_map_copy(ptr1->key_dt, ptr1->keys, ptr->keys, val_len);
    _array_map_copy(ptr1->val_dt, ptr1->values, ptr->values, val_len);
    memcpy(ptr->hashes, ptr1->hashes, init_len * sizeof (uint32));
    return ptr;
}
static int (Func_equals0)(IObjPtr src1, IObjPtr dst1){
    int ret = IObject_eqauls_base(src1, dst1);
    if(ret != kState_NEXT){
        return ret;
    }
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
                     dst->keys, (int)src->len_entry)){
        return kState_FAILED;
    }
    if(!_array_map_eq(src->val_dt, src->values,
                     dst->values, (int)src->len_entry)){
        return kState_FAILED;
    }
    return kState_OK;
}
static uint32 (Func_hash0)(IObjPtr src, uint32 hash){
    array_map_p ptr1 = (array_map_p)src;
    uint32 nhash = _array_map_hash(ptr1->key_dt, ptr1->keys,
                    ptr1->len_entry, hash);
    nhash = _array_map_hash(ptr1->val_dt, ptr1->values,
                        ptr1->len_entry, nhash);
    return nhash;
}
static void (Func_dump0)(IObjPtr src, hstring* hs){
     array_map_p ptr1 = (array_map_p)src;
    //_dump_target
    hstring_append(hs, "{");
    for(uint32 i = 0 ; i < ptr1->len_entry ; ++i){
        hstring_append(hs, "(");
        _dump_target(ptr1->key_dt, ptr1->keys, i, hs);
        hstring_append(hs, ":");
        _dump_target(ptr1->val_dt, ptr1->values, i, hs);
        hstring_append(hs, ")");
        if(i != ptr1->len_entry - 1){
            hstring_append(hs, ", ");
        }
    }
    hstring_append(hs, "}");
    //
    hstring_append(hs, "\n  hash = {");
    for(uint32 i = 0 ; i < ptr1->len_entry ; ++i){
         hstring_appendf(hs, "%u", ptr1->hashes[i]);
         if(i != ptr1->len_entry - 1){
             hstring_append(hs, ", ");
         }
    }
    hstring_append(hs, "}");
}
static void (Func_ref0)(IObjPtr src, int c){
    array_map_p ptr = (array_map_p)src;
    if(h_atomic_add(&ptr->baseObj.ref, c) == -c){
        _array_map_free(ptr->key_dt, ptr->keys, ptr->len_entry);
        _array_map_free(ptr->val_dt, ptr->values, ptr->len_entry);
        FREE(ptr->hashes);
        FREE_OBJ(ptr);
    }
}

DEF_IOBJ_INIT_CHILD(array_map, "$map")

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
    array_map_ensure_size(ptr, ptr->len_entry + 1);
    uint32 key_ele_size = dt_size(ptr->key_dt);
    uint32 val_ele_size = dt_size(ptr->val_dt);
    //handle hash.
    uint32 hash = __hash(ptr->key_dt, key, key_ele_size);
    if(ptr->len_entry == 0){
        memcpy(ptr->keys, key, key_ele_size);
        memcpy(ptr->values, value, val_ele_size);
        memcpy(ptr->hashes, &hash, sizeof (uint32));
        ptr->len_entry ++;
        return;
    }
    int pos = binarySearch_uint32(ptr->hashes, 0, (int)ptr->len_entry, hash);
    if(pos >= 0){
        void* val_dst = (char*)ptr->values + val_ele_size * (uint32)pos;
        //exist.
        if(oldVal){
            memcpy(oldVal, val_dst, val_ele_size);
        }else{
            //no copy out delete.
            dtype_obj_delete(&ptr->val_dt, val_dst);
        }
        memcpy(val_dst, value, val_ele_size);
    }else{
        // -(pos + 1)
        int pos1 = -(pos + 1);
        //arrays_insert(ptr->keys, ptr->len_entry, key_ele_size, key, pos1); DEF_ARRAY_INSERT_ONE
        arrays_insert(ptr->keys, ptr->len_entry, key_ele_size,  key, (uint32)pos1);
        arrays_insert(ptr->values, ptr->len_entry, val_ele_size, value, (uint32)pos1);
        arrays_insert(ptr->hashes, ptr->len_entry, sizeof(uint32), &hash, (uint32)pos1);
        ptr->len_entry ++;
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
    int pos = binarySearch_uint32(ptr->hashes, 0, (int)ptr->len_entry, hash);
    if(pos >= 0){
        void* val_dst = (char*)ptr->values + val_ele_size * (uint32)pos;
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
    int pos = binarySearch_uint32(ptr->hashes, 0, (int)ptr->len_entry, hash);
    if(pos >= 0){
        return (char*)ptr->values + val_ele_size * (uint32)pos;
    }
    return NULL;
}

int array_map_remove(array_map_p ptr, const void* key, void* oldKey, void* oldVal){
    ASSERT(key != NULL);
    uint32 key_ele_size = dt_size(ptr->key_dt);
    uint32 val_ele_size = dt_size(ptr->val_dt);
    //handle hash.
    uint32 hash = __hash(ptr->key_dt, key, key_ele_size);
    int pos = binarySearch_uint32(ptr->hashes, 0, (int)ptr->len_entry, hash);
    if(pos >= 0){
        void* val_dst = (char*)ptr->values + val_ele_size * (uint32)pos;
        if(oldVal){
            memcpy(oldVal, val_dst, val_ele_size);
        }else{
            dtype_obj_delete(&ptr->val_dt, val_dst);
        }
        void* key_dst = (char*)ptr->keys + key_ele_size * (uint32)pos;
        if(oldKey){
            memcpy(oldKey, key_dst, key_ele_size);
        }else{
            dtype_obj_delete(&ptr->key_dt, key_dst);
        }
        DEF_ARRAY_REMOVE_ONE(ptr->keys, ptr->len_entry, key_ele_size, pos);
        DEF_ARRAY_REMOVE_ONE(ptr->values, ptr->len_entry, val_ele_size, pos);
        DEF_ARRAY_REMOVE_ONE(ptr->hashes, ptr->len_entry, sizeof (uint32), pos);
        ptr->len_entry -- ;
        return 1;
    }
    return 0;
}


