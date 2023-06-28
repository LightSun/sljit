#ifndef H_ARRAY_H
#define H_ARRAY_H

#include "h7/common/dtypes.h"

struct array_list;
struct hstring;
struct VarArray;
typedef struct core_allocator core_allocator;
typedef struct harray harray;

struct harray{
    IObject baseObj;
    sint8 dt;
    sint8 free_data;
    struct VarArray* baseArr;           //the base array.
    struct array_list* ele_list; //for pointer array.
};

union harray_ele{
    sint8 _sint8;
    uint8 _uint8;
    sint16 _sint16;
    uint16 _uint16;
    sint32 _sint32;
    uint32 _uint32;
    sint64 _sint64;
    uint64 _uint64;
    float _float;
    double _double;
    void* _extra;  //can be harray or struct.
};
//-----------------------
DEF_IOBJ_CHILD_FUNCS(harray)

harray* harray_new(sint8 dt, int c);
harray* harray_new_nodata(sint8 dt, int c);

/**
new multi level array. like 'char arr[2][3][5]'
*/
harray* harray_new_multi(sint8 dt,
                         int* arr_count, int size);
//free_data: true to free data on recycle.
harray* harray_new_from_data(sint8 dt,
                             void* data, int data_size,
                             int ele_count, sint8 free_data);

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
harray* harray_new_chars2(const char* str, int len);

int harray_get_count(harray* arr);

void* harray_get_ptr_at(harray* p, int index);
int harray_geti(harray* arr, int index, union harray_ele* ptr);
int harray_seti(harray* arr, int index, union harray_ele* ptr);
int harray_seti2(harray* arr, int index, void* ptr);




#endif // H_ARRAY_H
