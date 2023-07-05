
#include <stdio.h>
#include <string.h>
#include "h7/var_array.h"
#include "h_array.h"
#include "h_list.h"
#include "h_string.h"
#include "h7/h_atomic.h"
#include "h7/common/halloc.h"
#include "h7/numbers.h"
#include "h7/hash.h"

#define MALLOC ALLOC

#define harray_dump_impl(hffi_t, type, format)\
case hffi_t:{\
    for(uint32 i = 0 ; i < arr->baseArr->ele_count ; i ++){\
        if(i != 0){hstring_append(hs, ",");}\
        hstring_appendf(hs, format, ((type*)arr->baseArr->data)[i]);\
    }\
    return;\
}break;
#define __HARRAY_TYPE_STR(ffi_t, type)\
case ffi_t: hstring_append(hs, type); break;

static inline void __harray_init(harray* arr);
//
static IObjPtr (Func_copy0)(IObjPtr src1, IObjPtr dst1){
    harray* src = (harray*)src1;
    harray* arr;
    if(dst1){
        arr = (harray*)dst1;
        //remove old
        if(arr->baseArr){
            VarArray_delete(arr->baseArr, arr->free_data);
        }
        if(arr->ele_list){
            array_list_delete(arr->ele_list, dtype_obj_delete,
                              &arr->dt);
        }
        //copy new
        if(src->baseArr){
            arr->baseArr = VarArray_copy(src->baseArr);
        }else{
            arr->baseArr = NULL;
        }
        if(src->ele_list){
            arr->ele_list = array_list_copy(src->ele_list,
                                    &arr->dt, dtype_obj_cpy);
        }else{
            arr->ele_list = NULL;
        }
    }else{
        arr = MALLOC(sizeof (harray));
        __harray_init(arr);
        if(src->baseArr){
            arr->baseArr = VarArray_copy(src->baseArr);
        }else{
            arr->baseArr = NULL;
        }
        if(src->ele_list){
            arr->ele_list = array_list_copy(src->ele_list,
                                    &arr->dt, dtype_obj_cpy);
        }else{
            arr->ele_list = NULL;
        }
    }
    arr->dt = src->dt;
    arr->free_data = 1;
    return arr;
}
static uint32 (Func_hash0)(IObjPtr src, uint32 seed){
    harray* arr = (harray*)src;
    if(arr->baseArr){
        return VarArray_hash(arr->baseArr, seed);
    }
    if(arr->ele_list){
        return array_list_hash(arr->ele_list,
                        &arr->dt, seed, dtype_obj_hash);
    }
    return seed;
}
static int (Func_equals0)(IObjPtr src1, IObjPtr dst1){
    int ret = IObject_eqauls_base(src1, dst1);
    if(ret != kState_NEXT){
        return ret;
    }
    harray* arr = (harray*)src1;
    harray* arr2 = (harray*)dst1;
    if(arr->dt != arr2->dt){
        return kState_FAILED;
    }
    if(dt_is_pointer(arr->dt)){
        switch (arr->dt) {
        case kType_P_STRING:
        case kType_P_MAP:
        case kType_P_ARRAY:
        case kType_P_OBJECT:
        case kType_P_FUNC:
            for(int i = 0, c = array_list_size(arr->ele_list) ;
                    i < c ; i ++){
                IObject* hobj = (IObject*)harray_get_ptr_at(arr, i);
                if(!hobj->Func_equals(
                            harray_get_ptr_at(arr,i),
                             harray_get_ptr_at(arr2,i))){
                    return kState_FAILED;
                }
            }
            break;
        default:
            LOGE("harray_eq >> unknown dtype = %d\n", arr->dt);
            return kState_FAILED;
        }
    }else{
        return VarArray_equals(arr->baseArr, arr2->baseArr, arr->dt);
    }
    return kState_OK;
}
static void (Func_dump0)(IObjPtr src1, hstring* hs){
    harray* arr = (harray*)src1;
    hstring_append(hs, "[");
    if(dt_is_pointer(arr->dt)){
        switch (arr->dt) {
        case kType_P_FUNC:
        case kType_P_FIELD:
        case kType_P_MAP:
        case kType_P_OBJECT:
        case kType_P_STRING:
        case kType_P_ARRAY:{
            for(int i = 0, c = array_list_size(arr->ele_list);
                        i < c ; i ++){
                if(i != 0){
                    hstring_append(hs, ",");
                }
                IObject* hobj = (IObject*)harray_get_ptr_at(arr, i);
                hobj->Func_dump(hobj, hs);
            }
        }break;
        }
    }else{
        DEF_DT_BASE_SWITCH_FORMAT(harray_dump_impl, arr->dt);
    }
    hstring_append(hs, "]");
}
static void Func_ref0(IObjPtr src1, int c){
    harray* arr = (harray*)src1;
    //printf("-- harray_delete >>> start delete harray: %p\n", arr);
    if(h_atomic_add(&arr->baseObj.ref, c) == -c){
        if(arr->ele_list){
            array_list_delete(arr->ele_list, dtype_obj_delete,
                              &arr->dt);
            arr->ele_list = NULL;
        }
        if(arr->baseArr){
            VarArray_delete(arr->baseArr, arr->free_data);
        }
        FREE(arr);
    }
}

DEF_IOBJ_INIT_CHILD(harray, "__$array")

//======================================================
harray* harray_new_arrays(struct array_list* arrays){
    harray* arr = MALLOC(sizeof (harray));
    __harray_init(arr);
    arr->baseArr = NULL;
    arr->dt = kType_P_ARRAY;
    arr->ele_list = arrays;
    arr->free_data = 0;
    return arr;
}
harray* harray_new(int dt, int initc){
    harray* arr = harray_new_nodata(dt, initc);
    if(!dt_is_pointer(dt)){
        arr->baseArr = VarArray_new(dt_size(dt), (uint32)(initc));
        arr->free_data = 1;
    }
    return arr;
}
//arr_count = [3,4,5]  means array. a[3][4][5]
static void setChildArray(harray* parent, int dt,
                          int* arr_counts, int size, int cur_index){
    int isLast = cur_index == size - 2;
    int c = arr_counts[cur_index];
    //a[3][4][5]
    array_list_ensure_size(parent->ele_list, c);
    for(int j = 0 ; j < c ; j ++){
        harray* harr_tmp = harray_new_multi(
                                  isLast ? dt : kType_P_ARRAY,
                                  arr_counts + 1,
                                  size - 1);
        array_list_set(parent->ele_list, j, harr_tmp);
    }
}
harray* harray_new_multi(int dt,
                         int* arr_count, int size){
    //multi level array
    int totalCount = 1;
    for(int i = 0 ; i < size ; i ++){
        totalCount *= arr_count[i];
    }
    harray* arr = MALLOC( sizeof (harray));
    __harray_init(arr);
    if(dt_is_pointer(dt)){
        arr->dt = kType_P_ARRAY;
        arr->ele_list = array_list_new2(8);
        arr->baseArr = NULL;
        setChildArray(arr, dt, arr_count, size, 0);
    }else{
        arr->dt = dt;
        arr->ele_list = NULL;
        arr->free_data = 1;
        arr->baseArr = VarArray_new_size(dt_size(dt), (uint32)totalCount);
    }
    return arr;
}
harray* harray_new_nodata(int dt, int initc){
    harray* arr = MALLOC( sizeof(harray));
    __harray_init(arr);
    arr->dt = dt;
    arr->baseArr = NULL;
    arr->free_data = 0;
    //element is pointer
    if(dt_is_pointer(dt)){
        arr->ele_list = array_list_new2(initc * 4 / 3 + 1);
    }else{
        arr->ele_list = NULL;
    }
    return arr;
}
//default as string
harray* harray_new_char(int count){
    harray* arr = harray_new(kType_S8, count);
    ((char*)arr->baseArr->data)[count - 1] = '\0';
    return arr;
}
harray* harray_new_chars(const char* str){
    int c = (int)strlen(str) + 1;
    harray* arr = harray_new(kType_S8, c);
    ((char*)arr->baseArr->data)[c - 1] = '\0';
    strcpy((char*)arr->baseArr->data, str);
    return arr;
}
harray* harray_new_chars2(const char* str, uint32 len){
    harray* arr = harray_new(kType_S8, (int)len + 1);
    memcpy(arr->baseArr->data, str, len);
    ((char*)arr->baseArr->data)[len] = '\0';
    return arr;
}
harray* harray_new_from_data(int dt,
                             void* data, uint32 data_size,
                             uint32 ele_count, sint8 free_data){
    harray* arr = MALLOC( sizeof (harray));
    __harray_init(arr);
    arr->baseArr = VarArray_new_from_data(
                dt_size(dt), data, data_size);
    arr->baseArr->ele_count = ele_count;
    arr->free_data = free_data;
    arr->dt = dt;
    arr->ele_list = NULL;
    return arr;
}
int harray_get_count(harray* arr){
    if(arr->baseArr){
        return (int)arr->baseArr->ele_count;
    }else{
        return array_list_size(arr->ele_list);
    }
}
void* harray_get_ptr_at(harray* p, int index){
    ASSERT(p->ele_list);
    return array_list_get(p->ele_list, index);
}

#define __GET_I(hffi_t, t)\
case hffi_t:{\
    ptr->_##t = ((t*)arr->baseArr->data)[index];\
}return kState_OK;

int harray_geti(harray* arr, int index, harray_ele* ptr){
    if(index < 0 || index >= harray_get_count(arr)){
        return kState_FAILED;
    }
    if(dt_is_pointer(arr->dt)){
        ptr->_extra = array_list_get(arr->ele_list, index);
        return kState_OK;
    }else{
        DEF_DT_BASE_SWITCH(__GET_I, arr->dt);
    }
    return kState_FAILED;
}

#define __SET_I(hffi_t, t)\
case hffi_t:{\
    ((t*)arr->baseArr->data)[index] = ptr->_##t;\
    /*if(hffi_t == HFFI_TYPE_SINT16) printf("__SET_I:  index = %d, val = %d\n", index, ((t*)arr->data)[index]); */\
}return kState_OK;

#define __SET_I_2(hffi_t, t)\
case hffi_t:{\
    ((t*)arr->baseArr->data)[index] = *((t*)ptr);\
}return kState_OK;

#define __ADD_I(hffi_t, t)\
case hffi_t:{\
    ((t*)arr->baseArr->data)[index] = *((t*)ptr);\
}return kState_OK;

int harray_seti(harray* arr, int index, harray_ele* ptr){
    if(index < 0 || index >= harray_get_count(arr)){
        return kState_FAILED;
    }
    if(dt_is_pointer(arr->dt)){
        void* p = array_list_get(arr->ele_list, index);
        if(p){
            dtype_obj_delete(&arr->dt, p);
        }
        array_list_set(arr->ele_list, index, ptr->_extra);
        return kState_OK;
    }else{
        DEF_DT_BASE_SWITCH(__SET_I, arr->dt);
    }
    return kState_FAILED;
}

int harray_set(harray* arr, int index, void* ptr){
    if(index >= harray_get_count(arr)){
        return kState_FAILED;
    }
    if(dt_is_pointer(arr->dt)){
        //remove old
        void* p = array_list_get(arr->ele_list, index);
        if(p){
            dtype_obj_delete(&arr->dt, p);
        }
        array_list_set(arr->ele_list, index, ptr);
        return kState_OK;
    }else{
        DEF_DT_BASE_SWITCH(__SET_I_2, arr->dt);
    }
    return kState_FAILED;
}

int harray_add(harray* arr, int index, void* ptr){
    int cur_c = harray_get_count(arr);
    if(index < 0 || index >= cur_c){
        index = cur_c;
    }
    if(dt_is_pointer(arr->dt)){
        array_list_addI(arr->ele_list, index, ptr);
    }else{
        VarArray_add(arr->baseArr, index, ptr);
    }
    return kState_OK;
}
int harray_indexOf(harray* arr, void* ptr){
    if(dt_is_pointer(arr->dt)){
        return array_list_index_of(arr->ele_list, ptr, &arr->dt, dtype_obj_equals);
    }else{
        return VarArray_indexOf(arr->baseArr, ptr, arr->dt);
    }
}
int harray_remove(harray* arr, void* ptr){
    if(dt_is_pointer(arr->dt)){
        void * ele = array_list_remove(arr->ele_list, ptr,  &arr->dt, dtype_obj_equals);
        if(ele){
            dtype_obj_delete(&arr->dt, ele);
            return kState_OK;
        }
    }else{
        int index = VarArray_indexOf(arr->baseArr, ptr, arr->dt);
        if(index >= 0){
            VarArray_remove_at(arr->baseArr, index);
            return kState_OK;
        }
    }
    return kState_FAILED;
}
int harray_remove_at(harray* arr, int index,harray_ele* ptr){
    if(dt_is_pointer(arr->dt)){
        void* p = array_list_remove_at(arr->ele_list, index);
        if(ptr){
            ptr->_extra = p;
        }else{
            dtype_obj_delete(&arr->dt, p);
        }
        return p != NULL;
    }else{
        if(ptr){
            VarArray_get2(arr->baseArr, index, arr->dt, ptr);
            return VarArray_remove_at(arr->baseArr, index);
        }else{
            return VarArray_remove_at(arr->baseArr, index);
        }
    }
}



