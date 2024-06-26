#ifndef H_ARRAY_H
#define H_ARRAY_H

#include "h7/common/dtypes.h"

struct array_list;
struct hstring;
struct VarArray;
typedef struct core_allocator core_allocator;
typedef union htype_value harray_ele;

typedef struct harray{
    IObject baseObj;
    int dt;
    sint8 free_data;
    struct VarArray* baseArr;    //the base array.
    struct array_list* ele_list; //for pointer array.
}harray, *harray_p;

//-----------------------
DEF_IOBJ_CHILD_FUNCS(harray)

harray* harray_new(int dt, int initc);
harray* harray_new_nodata(int dt, int initc);

/**
new multi level array. like 'char arr[2][3][5]'
*/
harray* harray_new_multi(int dt,
                         int* arr_count, int size);
//free_data: true to free data on recycle.
harray* harray_new_from_data(int dt,
                             void* data, uint32 data_size,
                             uint32 ele_count, sint8 free_data);

/**
 * @brief harray_new_array: create an array. element is array-ptr.
 * @param count
 * @return
 */
//every array type must be the same
harray* harray_new_arrays(struct array_list* arrays);
/**
 * @brief harray_new_array: create an array. element is char.
 * @param count
 * @return
 */
harray* harray_new_char(int count);
harray* harray_new_chars(const char* str);

//create array as fix length. and copy str to it.
harray* harray_new_chars2(const char* str, uint32 len);

int harray_get_count(harray* arr);
static inline int harray_size(harray* arr){
    return harray_get_count(arr);
}

void* harray_get_ptr_at(harray* p, int index);

int harray_geti(harray* arr, int index, harray_ele* ptr);
int harray_seti(harray* arr, int index, harray_ele* ptr);
int harray_set(harray* arr, int index, void* ptr);

int harray_add(harray* arr, int index, void* ptr);
static inline int harray_add2(harray* arr, void* ptr){
    return harray_add(arr, harray_get_count(arr), ptr);
}
int harray_remove(harray* arr, void* ptr);
//ptr: can be null.
int harray_remove_at(harray* arr, int index, harray_ele* ptr);

int harray_indexOf(harray* arr, void* ptr);


#endif // H_ARRAY_H
