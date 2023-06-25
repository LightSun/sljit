
#include <stdio.h>
#include <string.h>
#include "h_array.h"
#include "h7/atomic.h"
//#include "h_alloctor.h"
#include "h_list.h"
#include "h_string.h"
//#include "h_float_bits.h"
#include "h7/common/halloc.h"
#include "h7/numbers.h"

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
    hstring_append(hs, "]");\
    return;\
}break;
#define __HARRAY_TYPE_STR(ffi_t, type)\
case ffi_t: hstring_append(hs, type); break;

//
static IObjPtr (Func_copy0)(IObjPtr src1, IObjPtr dst1){
    harray* src = (harray*)src1;
    harray* arr;
    if(dst1){
        arr = (harray*)dst1;
    }else{
        arr = MALLOC(sizeof (harray));
    }
    arr->free_data = 1;
    arr->dt = src->dt;
    if(src->data){
        arr->data = MALLOC(src->data_size);
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
            if(!arr->baseObj.Func_equals(
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
    hstring_append(hs, "\n]");
}

//======================================================
harray* harray_new_arrays_nodata(struct array_list* arrays){
    int count = array_list_size(arrays);
    if(count == 0) return NULL;
    int every_size = ((harray*)array_list_get(arrays, 0))->data_size;
    harray* arr = MALLOC(sizeof (harray));
    arr->free_data = 1;
    arr->ele_count = count;
    arr->data_size = count * every_size;
    arr->data = NULL; //MALLOC(arr->data_size);
    arr->ref = 1;
    arr->dt = kType_P_ARRAY;
    arr->ele_list = MALLOC(sizeof (void*) * count);
    memset(arr->ele_list, 0, sizeof (sizeof (void*) * count));
    //ref array
    harray* _item;
    for(int i = 0 ; i < count ; i ++){
        _item = array_list_get(arrays, i);
        //ref to ele list
        harray_ref(_item, 1);
        arr->ele_list[i] = _item;
    }
    return arr;
}
harray* harray_new_arrays(struct array_list* arrays){
    DEF_HARRAY_ALLOC_DATA(harray_new_arrays_nodata(arrays))
    //
    int every_size = ((harray*)array_list_get(arrays, 0))->data_size;
    //copy data
    int startPos = 0;
    harray* _item;
    for(int i = 0 ; i < arr->ele_count ; i ++){
        _item = array_list_get(arrays, i);
        if(_item->data == NULL){
            harray_delete(arr);
            printf("harray with no data can't used to create harray.");
            return NULL;
        }
        memcpy(arr->data + startPos, _item->data, every_size);
        startPos += every_size;
    }
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
static void setChildArray(harray* parent,sint8 hffi_t,
                          int* arr_count, int size, int cur_index){
    int c, ele_count;
    int isLast = cur_index == size - 2;

    int offset = 0;
    harray* harr_tmp;
    c = arr_count[cur_index];
    //a[3][4][5]
    ele_count = arr_count[cur_index + 1];
    for(int j = 0 ; j < c ; j ++){
        harr_tmp = harray_new_from_data(parent->ca,
                                  isLast ? hffi_t : kType_P_ARRAY,
                                  parent->data + offset,
                                  parent->data_size / c,
                                  ele_count, 0);
        offset += parent->data_size / c;
        harray_seti2(parent, j, harr_tmp);
        if(!isLast){
            setChildArray(harr_tmp, hffi_t, arr_count,
                          size, cur_index + 1);
        }
        //-- ref
        harray_delete(harr_tmp);
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
    arr->free_data = 1;
    arr->dt = kType_P_ARRAY;
    arr->data_size = totalCount * dt_size(dt);
    arr->data = MALLOC(arr->data_size);
    arr->ele_count = arr_count[0];
    arr->ref = 1;

    //set sub harray.
    if(size > 1){
        setChildArray(arr, dt, arr_count, size, 0);
    }
    return arr;
}
harray* harray_new_nodata(sint8 dt, int count){
    int ele_size = dt_size(dt);
    harray* arr = MALLOC( sizeof(harray));
    arr->free_data = 1;
    arr->dt = dt;
    arr->data = NULL;
    arr->data_size = ele_size * count;
    arr->ele_count = count;
    arr->baseObj.ref = 1;
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
    int c = strlen(str) + 1;
    harray* arr = harray_new(kType_S8, len);
    ((char*)arr->data)[c - 1] = '\0';
    memcpy(arr->data, str, strlen(str));
    return arr;
}
harray* harray_new_from_data(sint8 dt,
                             void* data, int data_size,
                             int ele_count, sint8 free_data){
    harray* arr = MALLOC( sizeof (harray));
    arr->free_data = free_data;
    arr->data_size = data_size;
    arr->ele_count = ele_count;
    arr->dt = dt;
    arr->data = data;
    arr->ref = 1;
    return arr;
}
void harray_ensure_data(harray* src){
    if(!src->data){
        src->data = MALLOC2(src->ca, src->data_size);
        src->free_data = 1;
    }
}
harray* harray_copy(harray* src){
    harray* arr = MALLOC( sizeof (harray));
    arr->free_data = 1;
    arr->dt = src->dt;
    if(src->data){
        arr->data = MALLOC(src->data_size);
    }else{
        arr->data = NULL;
    }
    arr->data_size = src->data_size;
    arr->ele_count = src->ele_count;
    //no data.
    if(!arr->data){
        return arr;
    }
    //if(dt_is_pointer(src->p))
    if(src->ele_list){
        arr->ele_list = MALLOC(sizeof (void*) * src->ele_count);
        memset(arr->ele_list, 0, sizeof (void*) * src->ele_count);
        switch (src->hffi_t) {
        case HFFI_TYPE_HARRAY:
        {
            memcpy(arr->data, src->data, src->data_size);
            for(int i = 0 ; i < src->ele_count ; i ++){
                if(src->ele_list[i]){
                    arr->ele_list[i] = harray_copy((harray*)src->ele_list[i]);
                    HFFI_SET_PARENT(i,((harray*)src->ele_list[i]), arr, HFFI_TYPE_HARRAY);
                }else{
                    arr->ele_list[i] = NULL;
                }
            }
        }break;
        case HFFI_TYPE_HARRAY_PTR:
        {
            for(int i = 0 ; i < src->ele_count ; i ++){
                if(src->ele_list[i]){
                    ((void**)arr->data)[i] = arr->ele_list[i] = harray_copy((harray*)src->ele_list[i]);
                }else{
                    ((void**)arr->data)[i] = arr->ele_list[i] = NULL;
                }
            }
        }break;

        case HFFI_TYPE_STRUCT:{
            memcpy(arr->data, src->data, src->data_size);
            for(int i = 0 ; i < src->ele_count ; i ++){
                 if(src->ele_list[i]){
                     arr->ele_list[i] = hffi_struct_copy((struct hffi_struct*)src->ele_list[i]);
                     HFFI_SET_PARENT(i, ((struct hffi_struct*)src->ele_list[i]), arr, HFFI_TYPE_HARRAY);
                 }else{
                     arr->ele_list[i] = NULL;
                 }
            }
        }break;

        case HFFI_TYPE_STRUCT_PTR:{
            for(int i = 0 ; i < src->ele_count ; i ++){
                if(src->ele_list[i]){
                    ((void**)arr->data)[i] = arr->ele_list[i] = hffi_struct_copy((struct hffi_struct*)src->ele_list[i]);
                }else{
                    ((void**)arr->data)[i] = arr->ele_list[i] = NULL;
                }
            }
        }break;

        default:
            abort();//can't reach here
        }
    }else{
        memcpy(arr->data, src->data, src->data_size);
    }
    return arr;
}
static inline void __delete_ele(void* ud,void* ele){
    if(ele){
        int hffi_t = *((int*)ud);
        if(hffi_t == HFFI_TYPE_HARRAY || hffi_t == HFFI_TYPE_HARRAY_PTR){
            harray_delete((harray*)ele);
        }else if(hffi_t == HFFI_TYPE_STRUCT || hffi_t == HFFI_TYPE_STRUCT_PTR){
            hffi_delete_struct((struct hffi_struct*)ele);
        }
    }
}
void harray_delete(harray* arr){
    if(atomic_add(&arr->ref, -1) == 1){
        //printf("-- harray_delete >>> start delete harray: %p\n", arr);
        int hffi_t = arr->hffi_t;
        if(arr->ele_list){
            for(int i = 0 ; i < arr->ele_count ; i ++){
                if(arr->ele_list[i]){
                    __delete_ele(&hffi_t, arr->ele_list[i]);
                }
            }
            FREE(arr->ele_list);
        }
        if(arr->free_data && arr->data){
            FREE(arr->data);
        }
        HFFI_FREE_PARENT(arr)
        FREE(arr);
    }
}
void harray_ref(harray* arr, int c){
    atomic_add(&arr->ref, c);
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
    switch (arr->hffi_t) {
        case HFFI_TYPE_POINTER:{
             ptr->_extra = ((void**)arr->data)[index];
             return HFFI_STATE_OK;
        }break;
        case HFFI_TYPE_STRUCT_PTR:
        case HFFI_TYPE_HARRAY_PTR:{
            if(arr->ele_list == NULL){
                return HFFI_STATE_FAILED;
            }
            void *data = arr->ele_list[index];
            if(data){
                ptr->_extra = data;
                //printf("get harray: index = %d, arr_addr = %p\n", index, ptr->_extra);
                return HFFI_STATE_OK;
            }
        }break;

        case HFFI_TYPE_HARRAY:{
            //find old
            void *data =  arr->ele_list[index];
            if(data){
                ptr->_extra = data;
                //must provide an array to copy daya.
                int target_size = ((harray*)ptr->_extra)->data_size;
                void* data_ptr = arr->data + index * (arr->data_size / arr->ele_count);
                memcpy(((harray*)ptr->_extra)->data, data_ptr, target_size);
                return HFFI_STATE_OK;
            }else{
                //if not set. we create a new array but data from parent ?.
//              harr_tmp = harray_new_from_data(isLast ? hffi_t : HFFI_TYPE_HARRAY,
//                                     parent->data + offset, parent->data_size / c, ele_count, 0);
            }
        }break;

        case HFFI_TYPE_STRUCT:{
            //find old
            void *data = arr->ele_list[index];
            if(data){
                ptr->_extra = data;
                int target_size = hffi_struct_get_data_size((struct hffi_struct*)ptr->_extra);
                void* data_ptr = arr->data + index * (arr->data_size / arr->ele_count);
                memcpy(hffi_struct_get_data((struct hffi_struct*)ptr->_extra), data_ptr, target_size);
                return HFFI_STATE_OK;
            }
        }break;
    }
    return HFFI_STATE_FAILED;
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
    switch (arr->dt) {
        case kType_P_ARRAY:
        case kType_P_MAP:
        case kType_P_OBJECT:
        case kType_P_STRING:
        case kType_P_FUNC:
        {
            harray* o = (harray*)ptr->_extra;
            harray_ref(o, 1);
            ((void**)arr->data)[index] = o;
        }
        return kState_OK;
    }

    return kState_FAILED;
}

int harray_seti2(harray* arr, int index, void* ptr){
    if(!arr->data || index >= arr->ele_count){
        return kState_FAILED;
    }
    DEF_DT_BASE_SWITCH(__SET_I_2, arr->dt);
    switch (arr->dt) {
        case kType_P_ARRAY:
        case kType_P_MAP:
        case kType_P_OBJECT:
        case kType_P_STRING:
        case kType_P_FUNC:
        {
            harray* o = (harray*)ptr;
            harray_ref(o, 1);
            ((void**)arr->data)[index] = o;
        }
        return kState_OK;
    }

    return kState_FAILED;
}

int harray_eq(harray* arr, harray* arr2){
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

    case kType_P_MAP:
    case kType_P_ARRAY:
    case kType_P_OBJECT:
        for(int i = 0, c = arr->ele_count ; i < c ; i ++){
            if(!harray_eq((harray*)arr->data, (harray*)arr2->data)){
                return kState_FAILED;
            }
        }
        break;

    case kType_P_STRING:
    case kType_P_FUNC:
        if(memcmp(arr->data, arr2->data, arr->data_size) != 0){
            return kState_FAILED;
        }
        break;
    }
    return kState_OK;
}



void harray_dump(harray* arr, struct hstring* hs){

    hstring_appendf(hs, "[ desc: count = %d, ele_size = %d, ele_type = ",
                    arr->ele_count, arr->data_size / arr->ele_count);
    DEF_DT_ALL_SWITCH(__HARRAY_TYPE_STR, arr->dt)
    hstring_append(hs, " ]\n data: [");

    DEF_DT_BASE_SWITCH_FORMAT(harray_dump_impl, arr->dt)
    switch (arr->dt) {
        case kType_P_FUNC:{
            hstring_append(hs, "\n<function> ");
        }break;
        case kType_P_ARRAY:{
            hstring_append(hs, "\n<array> ");
            union harray_ele ele;
            for(int i = 0 ; i < arr->ele_count ; i ++){
                harray_geti(arr, i, &ele);
                if(ele._extra != NULL){
                    harray_dump((harray*)ele._extra, hs);
                }else{
                    hstring_append(hs, "null");
                }
                if(i != 0){
                    hstring_append(hs, ", ");
                }
            }
        }break;
    }
    hstring_append(hs, "]");
}

int harray_set_struct_ptr(harray* arr, int index, struct hffi_struct* str){
    if(arr->hffi_t != HFFI_TYPE_STRUCT_PTR || index >= arr->ele_count){
        return HFFI_STATE_FAILED;
    }
    if(arr->ele_list[index]){
        hffi_delete_struct((struct hffi_struct*)arr->ele_list[index]);
    }
    arr->ele_list[index] = str;
    if(arr->data){
        ((void**)arr->data)[index] = str;
    }
    return HFFI_STATE_OK;
}
int harray_set_harray_ptr(harray* arr, int index, harray* str){
    if(arr->hffi_t != HFFI_TYPE_HARRAY_PTR || index >= arr->ele_count){
        return HFFI_STATE_FAILED;
    }
    if(arr->ele_list[index]){
        harray_delete((harray*)arr->ele_list[index]);
    }
    arr->ele_list[index] = str;
    if(arr->data){
        ((void**)arr->data)[index] = str;
    }
    return HFFI_STATE_OK;
}

//int* a = ...
//void* ptr = &a;
int harray_set_all(harray* arr, void* ptr){

#define HARRAY_COPY_DATA()\
if(arr->data){\
    if(ptr){\
        memcpy(arr->data, ptr, arr->data_size);\
    }\
}else{\
    /* must ensure the data size match */\
    arr->data = ptr;\
}
    switch (arr->hffi_t) {

    case HFFI_TYPE_SINT8:
    case HFFI_TYPE_UINT8:
    case HFFI_TYPE_SINT16:
    case HFFI_TYPE_UINT16:
    case HFFI_TYPE_SINT32:
    case HFFI_TYPE_UINT32:
    case HFFI_TYPE_SINT64:
    case HFFI_TYPE_UINT64:
    case HFFI_TYPE_FLOAT:
    case HFFI_TYPE_DOUBLE:
    case HFFI_TYPE_INT:
    case HFFI_TYPE_POINTER:{
        HARRAY_COPY_DATA()
        return HFFI_STATE_OK;
    }break;

    case HFFI_TYPE_HARRAY:{
        HARRAY_COPY_DATA()
        if(arr->data){
            for(int i = 0 ; i< arr->ele_count ; i ++){
                if(arr->ele_list[i] != NULL){
                    if(harray_set_all((harray*)arr->ele_list[i],
                                      ((void**)arr->data)[i]) == HFFI_STATE_FAILED){
                        return HFFI_STATE_FAILED;
                    }
                }
            }
            return HFFI_STATE_OK;
        }
    }break;
    case HFFI_TYPE_STRUCT:
    {
        HARRAY_COPY_DATA()
        if(arr->data){
            for(int i = 0 ; i< arr->ele_count ; i ++){
                if(arr->ele_list[i] != NULL){
                    if(hffi_struct_set_all((struct hffi_struct*)arr->ele_list[i],
                                           ((void**)arr->data)[i]) == HFFI_STATE_FAILED){
                        return HFFI_STATE_FAILED;
                    }
                }
            }
            return HFFI_STATE_OK;
        }
    }break;

    case HFFI_TYPE_HARRAY_PTR:{
        if(!arr->data){
            arr->data = MALLOC(sizeof (void*) * arr->ele_count);
            arr->free_data = 1;
        }
        void** data = arr->data;
        for(int i = 0 ; i< arr->ele_count ; i ++){
            if(arr->ele_list[i] == NULL){
                return HFFI_STATE_FAILED;
            }
            if(harray_set_all((harray*)arr->ele_list[i],
                              ((void**)arr->data)[i]) == HFFI_STATE_FAILED){
                return HFFI_STATE_FAILED;
            }
            //set pointer
            data[i] = ((harray*)arr->ele_list[i])->data;
        }
        return HFFI_STATE_OK;
    }break;

    case HFFI_TYPE_STRUCT_PTR:{
        if(!arr->data){
            arr->data = MALLOC(sizeof (void*) * arr->ele_count);
            arr->free_data = 1;
        }
        void** data = arr->data;
        for(int i = 0 ; i< arr->ele_count ; i ++){
            if(arr->ele_list[i] == NULL){
                return HFFI_STATE_FAILED;
            }
            if(hffi_struct_set_all((struct hffi_struct*)arr->ele_list[i],
                                   ((void**)arr->data)[i]) == HFFI_STATE_FAILED){
               return HFFI_STATE_FAILED;
            }
            data[i] = hffi_struct_get_data((struct hffi_struct*)arr->ele_list[i]);
        }
        return HFFI_STATE_OK;
    }break;

    }
    return HFFI_STATE_FAILED;
}


