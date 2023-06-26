
#include <stdio.h>
#include <string.h>
#include "h_array.h"
#include "h_list.h"
#include "h_string.h"
#include "h7/h_atomic.h"
#include "h7/common/halloc.h"
#include "h7/numbers.h"
#include "h7/hash.h"

#define DEF_HARRAY_ALLOC_DATA(x)\
harray* arr = x;\
if(arr == NULL) return NULL;\
arr->data = ALLOC(arr->data_size);\
memset(arr->data, 0, arr->data_size);\
arr->free_data = 1;

#define MALLOC ALLOC

#define harray_dump_impl(hffi_t, type, format)\
case hffi_t:{\
    for(int i = 0 ; i < arr->ele_count ; i ++){\
        if(i != 0){hstring_append(hs, ", ");}\
        hstring_appendf(hs, format, ((type*)arr->data)[i]);\
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
    }else{
        arr = MALLOC(sizeof (harray));
        __harray_init(arr);
    }
    arr->dt = src->dt;
    arr->free_data = 1;
    if(src->data){
        arr->data = MALLOC(src->data_size);
        memcpy(arr->data, src->data, src->data_size);
    }else{
        arr->data = NULL;
    }

    arr->data_size = src->data_size;
    arr->ele_count = src->ele_count;
    if(src->ele_list){
        arr->ele_list = array_list_copy(src->ele_list,
                                dtype_obj_cpy, &arr->dt);
    }else{
        arr->ele_list = NULL;
    }
    return arr;
}
static uint32 (Func_hash0)(IObjPtr src, uint32 seed){
    harray* arr = (harray*)src;
    if(arr->data){
        return fasthash32(arr->data, arr->data_size, seed);
    }
    if(arr->ele_list){
        return array_list_hash(arr->ele_list, dtype_obj_hash,
                        &arr->dt, seed);
    }
    return seed;
}
static int (Func_equals0)(IObjPtr src1, IObjPtr dst1){
    harray* arr = (harray*)src1;
    harray* arr2 = (harray*)dst1;
    if(arr == arr2){
        return kState_OK;
    }
    if(arr->dt != arr2->dt || arr->ele_count != arr2->ele_count
            || arr->data_size != arr2->data_size){
        return kState_FAILED;
    }
    if(!arr->data && !arr2->data){
        return kState_OK;
    }else if((!arr->data && arr2->data) || (arr->data && !arr2->data)){
        return kState_FAILED;
    }

    switch (arr->dt) {
    case kType_S8:
    case kType_U8:
    case kType_S16:
    case kType_U16:
    case kType_S32:
    case kType_U32:
    case kType_S64:
    case kType_U64:
        if(memcmp(arr->data, arr2->data, arr->data_size) != 0){
            return kState_FAILED;
        }
        break;
    case kType_F32:
        for(int i = 0, c = arr->ele_count ; i < c ; i ++){
            if(!isFloatEquals(((float*)arr->data)[i], ((float*)arr2->data)[i])){
                return kState_FAILED;
            }
        }
        break;
    case kType_F64:
        for(int i = 0, c = arr->ele_count ; i < c ; i ++){
            //printf("arr_eq double:  i = %d, v1 = %.5f, v2 = %.5f\n", i , ((float*)arr->data)[i], ((float*)arr2->data)[i]);
            if(!isDoubleEquals(((double*)arr->data)[i], ((double*)arr2->data)[i])){
                return kState_FAILED;
            }
        }
        break;
    case kType_VOID:
        return kState_FAILED;

    case kType_P_STRING: //TODO different?
    case kType_P_MAP:
    case kType_P_ARRAY:
    case kType_P_OBJECT:
        for(int i = 0, c = arr->ele_count ; i < c ; i ++){
            IObject* hobj = (IObject*)get_ptr_at(arr, i);
            if(!hobj->Func_equals(
                        get_ptr_at(arr,i),
                         get_ptr_at(arr2,i))){
                return kState_FAILED;
            }
        }
        break;

    case kType_P_FUNC:
        if(memcmp(arr->data, arr2->data, arr->data_size) != 0){
            return kState_FAILED;
        }
        break;
    }
    return kState_OK;
}
static void (Func_dump0)(IObjPtr src1, hstring* hs){
    harray* arr = (harray*)src1;
    hstring_appendf(hs, "[ desc: count = %d, ele_size = %d, ele_type = ",
                    arr->ele_count, arr->data_size / arr->ele_count);
    DEF_DT_ALL_SWITCH(__HARRAY_TYPE_STR, arr->dt)
    hstring_append(hs, " ], data: [\n");

    DEF_DT_BASE_SWITCH_FORMAT(harray_dump_impl, arr->dt);
    if(dt_is_pointer(arr->dt)){
        switch (arr->dt) {
        case kType_P_FUNC:{
            for(int i = 0 ; i < arr->ele_count ; i ++){
                if(i != 0){
                    hstring_append(hs, ", ");
                }
                hstring_appendf(hs, "func<%p>",
                            array_list_get(arr->ele_list, i));
            }
        }break;
        case kType_P_MAP:
        case kType_P_OBJECT:
        case kType_P_STRING:
        case kType_P_ARRAY:{
            for(int i = 0 ; i < arr->ele_count ; i ++){
                if(i != 0){
                    hstring_append(hs, ", ");
                }
                IObject* hobj = (IObject*)get_ptr_at(arr, i);
                hobj->Func_dump(hobj, hs);
            }
        }break;
        }
    }
    hstring_append(hs, "\n]");
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
        if(arr->free_data && arr->data){
            FREE(arr->data);
        }
        FREE(arr);
    }
}

static inline void __harray_init(harray* arr){
    arr->baseObj.ref = 1;
    arr->baseObj.Func_copy = Func_copy0;
    arr->baseObj.Func_dump = Func_dump0;
    arr->baseObj.Func_equals = Func_equals0;
    arr->baseObj.Func_hash = Func_hash0;
    arr->baseObj.Func_ref = Func_ref0;
}

//======================================================
harray* harray_new_arrays(struct array_list* arrays){
    int count = array_list_size(arrays);
    harray* arr = MALLOC(sizeof (harray));
    __harray_init(arr);
    arr->free_data = 1;
    arr->ele_count = count;
    arr->data_size = count * sizeof(void*);//element is pointer.
    arr->data = NULL;
    arr->dt = kType_P_ARRAY;
    arr->ele_list = arrays;
    return arr;
}
void* get_ptr_at(harray* p, int index){
    ASSERT(p->ele_list);
    return array_list_get(p->ele_list, index);
}
harray* harray_new(sint8 dt, int c){
    DEF_HARRAY_ALLOC_DATA(harray_new_nodata(dt, c))
    return arr;
}
//arr_count = [3,4,5]  means array. a[3][4][5]
static void setChildArray(harray* parent,sint8 dt,
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
harray* harray_new_multi(sint8 dt,
                         int* arr_count, int size){
    //multi level array
    int totalCount = 1;
    for(int i = 0 ; i < size ; i ++){
        totalCount *= arr_count[i];
    }
    harray* arr = MALLOC( sizeof (harray));
    __harray_init(arr);
    arr->free_data = 1;
    arr->dt = kType_P_ARRAY;
    arr->data_size = totalCount * dt_size(dt);
    arr->ele_count = arr_count[0];
    if(dt_is_pointer(dt)){
        arr->ele_list = array_list_new2(8);
        arr->data = NULL;
        setChildArray(arr, dt, arr_count, size, 0);
    }else{
        arr->ele_list = NULL;
        arr->data = MALLOC(arr->data_size);
    }
    return arr;
}
harray* harray_new_nodata(sint8 dt, int count){
    int ele_size = dt_size(dt);
    harray* arr = MALLOC( sizeof(harray));
    __harray_init(arr);
    arr->free_data = 1;
    arr->dt = dt;
    arr->data = NULL;
    arr->data_size = ele_size * count;
    arr->ele_count = count;
    //element is pointer
    if(dt_is_pointer(dt)){
        arr->ele_list = array_list_new2(count * 4 / 3 + 1);
    }else{
        arr->ele_list = NULL;
    }
    return arr;
}
//default as string
harray* harray_new_char(int count){
    harray* arr = harray_new(kType_S8, count);
    ((char*)arr->data)[count - 1] = '\0';
    return arr;
}
harray* harray_new_chars(const char* str){
    int c = strlen(str) + 1;
    harray* arr = harray_new(kType_S8, c);
    ((char*)arr->data)[c - 1] = '\0';
    strcpy((char*)arr->data, str);
    return arr;
}
harray* harray_new_chars2(const char* str, int len){
    harray* arr = harray_new(kType_S8, len + 1);
    memcpy(arr->data, str, len);
    ((char*)arr->data)[len] = '\0';
    return arr;
}
harray* harray_new_from_data(sint8 dt,
                             void* data, int data_size,
                             int ele_count, sint8 free_data){
    harray* arr = MALLOC( sizeof (harray));
    __harray_init(arr);
    arr->free_data = free_data;
    arr->data_size = data_size;
    arr->ele_count = ele_count;
    arr->dt = dt;
    arr->data = data;
    arr->ele_list = NULL;
    return arr;
}
void harray_ensure_data(harray* src){
    if(!dt_is_pointer(src->dt)){
        if(!src->data){
            src->data = MALLOC(src->data_size);
            src->free_data = 1;
        }
    }
}

#define __GET_I(hffi_t, t)\
case hffi_t:{\
    ptr->_##t = ((t*)arr->data)[index];\
}return kState_OK;

int harray_geti(harray* arr, int index, union harray_ele* ptr){
    if(index >= arr->ele_count){
        return kState_FAILED;
    }
    DEF_DT_BASE_SWITCH(__GET_I, arr->dt);
    if(dt_is_pointer(arr->dt)){
        ptr->_extra = array_list_get(arr->ele_list, index);
        return kState_OK;
    }
    return kState_FAILED;
}

#define __SET_I(hffi_t, t)\
case hffi_t:{\
    ((t*)arr->data)[index] = ptr->_##t;\
    /*if(hffi_t == HFFI_TYPE_SINT16) printf("__SET_I:  index = %d, val = %d\n", index, ((t*)arr->data)[index]); */\
}return kState_OK;

#define __SET_I_2(hffi_t, t)\
case hffi_t:{\
    ((t*)arr->data)[index] = *((t*)ptr);\
}return kState_OK;

int harray_seti(harray* arr, int index, union harray_ele* ptr){
    if(!arr->data || index >= arr->ele_count){
        return kState_FAILED;
    }
    DEF_DT_BASE_SWITCH(__SET_I, arr->dt);
    if(dt_is_pointer(arr->dt)){
        array_list_set(arr->ele_list, index, ptr->_extra);
        return kState_OK;
    }
    return kState_FAILED;
}

int harray_seti2(harray* arr, int index, void* ptr){
    if(!arr->data || index >= arr->ele_count){
        return kState_FAILED;
    }
    DEF_DT_BASE_SWITCH(__SET_I_2, arr->dt);
    if(dt_is_pointer(arr->dt)){
        array_list_set(arr->ele_list, index, ptr);
        return kState_OK;
    }
    return kState_FAILED;
}



