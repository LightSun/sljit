#ifndef VAR_ARRAY_H
#define VAR_ARRAY_H

#include "h7/common/dtypes.h"
#include "h7/common/halloc.h"
#include "h7/numbers.h"

typedef struct VarArray VarArray;
struct VarArray{
    void* data;
    uint32 alloc;
    uint32 ele_size; //the every size of element.
    uint32 ele_count;
    float factor; //the load factor;
};

static inline int _VarArray_alloc_ele_len(VarArray* list){
    return list->alloc / list->ele_size;
}
static inline uint32 VarArray_valid_data_size(VarArray* list){
    return list->ele_size * list->ele_count;
}

static inline void __grow(VarArray* list, int minCapacity){
   int oldCapacity = _VarArray_alloc_ele_len(list);
   int newCapacity = oldCapacity + (oldCapacity >> 1);
   if (newCapacity - minCapacity < 0){
       newCapacity = minCapacity;
   }
   list->data = REALLOC(list->data, list->alloc,
                       newCapacity * list->ele_size);
   list->alloc = newCapacity * list->ele_size;
}
static inline void VarArray_ensure_capacity(VarArray* list, uint32 cap){
    if(_VarArray_alloc_ele_len(list) < (int)cap){
        __grow(list, (int)cap);
    }
}
static inline void VarArray_delete(VarArray* list,int free_data){
    if(free_data){
        FREE(list->data);
    }
    FREE(list);
}

static inline VarArray* VarArray_new(uint32 ele_size,uint32 init_len){
    VarArray* p = (VarArray*)ALLOC(sizeof(struct VarArray));
    p->alloc = init_len * ele_size;
    p->data = ALLOC(p->alloc);
    p->ele_size = ele_size;
    p->ele_count = 0;
    p->factor = DEFAULT_LOAD_FACTOR;
}

static inline VarArray* VarArray_new_size(uint32 ele_size,
                                          uint32 size){
    VarArray* p = VarArray_new(ele_size, size / DEFAULT_LOAD_FACTOR + 1);
    p->ele_count = size;
    return p;
}

static inline VarArray* VarArray_new_from_data(uint32 ele_size,
                                          void* data,
                                          uint32 dsize){
    VarArray* p = (VarArray*)ALLOC(sizeof(struct VarArray));
    p->alloc = dsize;
    p->data = data;
    p->ele_size = ele_size;
    p->ele_count = 0;
    p->factor = DEFAULT_LOAD_FACTOR;
    return p;
}

static inline VarArray* VarArray_copy(VarArray* src){
    VarArray* p = (VarArray*)ALLOC(sizeof(struct VarArray));
    p->alloc = src->alloc;
    p->data = ALLOC(p->alloc);
    p->ele_size = src->ele_size;
    p->ele_count = src->ele_count;
    p->factor = src->factor;
    memcpy(p->data, src->data, src->alloc);
    return p;
}
//-1 means last. index
static inline void VarArray_add(VarArray* p,int index, void* sd){
    if(index < 0) index = p->ele_count;
    VarArray_ensure_capacity(p, 1);
    int offset = index * p->ele_size;
    char* data = (char*)p->data + offset;
    if(index < (int)p->ele_count){
        //move old. c = 4, index = 2. move_c = c - index
        int move_c = (int)p->ele_count - index;
        memmove(data + p->ele_size * move_c, data, p->ele_size * move_c);
    }
    memcpy(data, sd, p->ele_size);
    p->ele_count ++;
}
static inline void VarArray_set(VarArray* p,int index, void* sd){
    ASSERT(index >= 0 && index < (int)p->ele_count);
    char* data = (char*)p->data + index * p->ele_size;
    memcpy(data, sd, p->ele_size);
}

static inline void VarArray_remove(VarArray* p,int index){
    ASSERT(index >= 0 && index < (int)p->ele_count);
    char* data = (char*)p->data + index * p->ele_size;
    int move_c = (int)p->ele_count - index - 1;
    memmove(data, data + p->ele_size * move_c, p->ele_size * move_c);
}

static inline void VarArray_get(VarArray* p,int index, void* out_d){
    ASSERT(index >= 0 && index < (int)p->ele_count);
    char* data = (char*)p->data + index * p->ele_size;
    memcpy(out_d, data, p->ele_size);
}

static inline int VarArray_equals_float_data(VarArray* p1, VarArray* p2){
    if(p1->ele_count != p2->ele_count){
        return kState_FAILED;
    }
    if(p1->ele_size != p2->ele_size){
        return kState_FAILED;
    }
    for(int i = 0, c = p1->ele_count ; i < c ; i ++){
        if(!isFloatEquals(((float*)p1->data)[i],
                          ((float*)p2->data)[i])){
            return kState_FAILED;
        }
    }
    return kState_OK;
}
static inline int VarArray_equals_double_data(VarArray* p1, VarArray* p2){
    if(p1->ele_count != p2->ele_count){
        return kState_FAILED;
    }
    if(p1->ele_size != p2->ele_size){
        return kState_FAILED;
    }
    for(int i = 0, c = p1->ele_count ; i < c ; i ++){
        if(!isDoubleEquals(((double*)p1->data)[i],
                          ((double*)p2->data)[i])){
            return kState_FAILED;
        }
    }
    return kState_OK;
}
static inline int VarArray_equals(VarArray* p1, VarArray* p2, int dt){
    int ret = IObject_eqauls_base(p1, p2);
    if(ret != kState_NEXT){
        return ret;
    }
    if(dt == kType_F32){
        return VarArray_equals_float_data(p1, p2);
    }
    if(dt == kType_F64){
        return VarArray_equals_double_data(p1, p2);
    }
    if(memcmp(p1->data, p2->data, p1->ele_count * p1->ele_size) != 0){
        return kState_FAILED;
    }
    return kState_OK;
}
#endif // VAR_ARRAY_H
