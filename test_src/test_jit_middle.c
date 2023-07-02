#include "h7/h_string.h"
#include "h7/var_array.h"
#include "h7/h_list.h"
#include "h7/binary_search.h"

#include "h7/utf8_string.h"
#include "h7/h_array.h"
#include "h7/array_map.h"
#include "h7/h_field.h"
#include "h7/h_method.h"
#include "h7/h_object.h"

#define ASSERT_EQ_D(v1, v2)\
    if(!(isDoubleEquals(v1, v2))){\
        fprintf(stderr, "assert failed. file = %s, func = %s, line = %d, \n", \
            __FILE__, __FUNCTION__,__LINE__);\
        fprintf(stderr, "v1, v2= %g, %g\n", v1, v2);\
        abort();\
    }

#define ASSERT_EQ_F(v1, v2)\
    if(!(isFloatEquals(v1, v2))){\
        fprintf(stderr, "assert failed. file = %s, func = %s, line = %d\n", \
            __FILE__, __FUNCTION__,__LINE__);\
        fprintf(stderr, "v1, v2= %g, %g\n", v1, v2);\
        abort();\
    }

#define ASSERT_EQ_INT(v1, v2)\
    if(!(v1 == v2)){\
        fprintf(stderr, "assert failed. file = %s, func = %s, line = %d\n", \
            __FILE__, __FUNCTION__,__LINE__);\
        fprintf(stderr, "v1, v2= %d, %d\n", v1, v2);\
        abort();\
    }

static void test_binary_search();
static void test_var_array();
static void test_array_list();

static void test_harray_int();
static void test_harray_float();
static void test_harray_ptr();

static void test_array_map();

static void _free_VarArray(void* ele){
    VarArray_p p = (VarArray_p)ele;
    VarArray_delete(p, 1);
}

static inline hfield_p new_field_int(int val){
    return hfield_new2("", kType_S32, &val);
}

void test_jit_middle_all(){
    //test_binary_search();
    test_var_array();
    test_array_list();
    test_harray_int();
    test_harray_float();
    test_harray_ptr();
    test_array_map();
}
//----------------------

void test_array_map(){
    int dt = kType_P_MAP;
    array_map_p p = array_map_new(kType_S32, kType_F64, 10);

    int loop_c = 10;
    for(int i = 0 ; i < loop_c ; i ++){
        int key = i;
        double val = (double)key * 1.11f;
        array_map_put(p, &key, &val, NULL);
        ASSERT(array_map_size(p) == i + 1);
        //dtype_obj_log(&dt, p);
        double val2 = 0;
        ASSERT(array_map_get(p, &key, &val2));
        ASSERT_EQ_D(val, val2);
    }
    int idx = 8;
    double val;
    void* vp = array_map_rawget(p, &idx);
    val = *(double*)vp;
    ASSERT_EQ_D(val, 8.88);

    array_map_remove(p, &idx, NULL,NULL);
    ASSERT(array_map_size(p) == loop_c - 1);
    //dtype_obj_log(&dt, p);

    array_map_delete(p);
}

void test_harray_ptr(){
    int dt = kType_P_ARRAY;
    harray_p p = harray_new(kType_P_FIELD, 8);
    int loop_c = 10;
    for(int i = 0 ; i < loop_c ; i ++){
        hfield_p f = new_field_int(i);
        harray_add(p, i, f);
        ASSERT(harray_get_count(p) == i + 1);
        harray_ele ele;
        harray_geti(p, i, &ele);
        ASSERT(ele._extra == f);
    }
    int val = 10086;
    hfield_p f = new_field_int(val);
    harray_set(p, 1, f);
    harray_ele ele;
    harray_geti(p, 1, &ele);
    ASSERT(ele._extra == f);

    //dtype_obj_log(&dt, p);

    //ref + 1
    hfield_ref(f);
    //ref -1
    harray_remove(p, f);
    ASSERT(harray_get_count(p) == loop_c - 1);

    //dtype_obj_log(&dt, p);

    harray_add(p, -1, f);
    //dtype_obj_log(&dt, p);
    ASSERT(harray_get_count(p) == loop_c);

    int index = harray_indexOf(p, f);
    ASSERT(index == loop_c - 1);

    harray_remove_at(p, 1, NULL); //auto delete hfield_p
    ASSERT(harray_get_count(p) == loop_c - 1);

    harray_delete(p);
}

void test_harray_float(){
    int dt = kType_F32;
    harray_p p = harray_new(dt, 8);
    int loop_c = 10;
    for(int i = 0 ; i < loop_c ; i ++){
        int val = i;
        harray_add(p, i, &val);
        ASSERT(harray_get_count(p) == i + 1);
        harray_ele ele;
        harray_geti(p, i, &ele);
        ASSERT(ele._sint32 == val);
    }
    float val = 100.86f;
    harray_set(p, 1, &val);
    harray_ele ele;
    harray_geti(p, 1, &ele);
    ASSERT(isFloatEquals(ele._float, val));

    harray_remove(p, &val);
    ASSERT(harray_get_count(p) == loop_c - 1);

    harray_add(p, -1, &val);
    ASSERT(harray_get_count(p) == loop_c);

    int index = harray_indexOf(p, &val);
    ASSERT(index == loop_c - 1);

    harray_remove_at(p, 1, &ele);
    ASSERT(harray_get_count(p) == loop_c - 1);

    harray_delete(p);
}

void test_harray_int(){
    int dt = kType_S32;
    harray_p p = harray_new(dt, 8);
    int loop_c = 10;
    for(int i = 0 ; i < loop_c ; i ++){
        int val = i;
        harray_add(p, i, &val);
        ASSERT(harray_get_count(p) == i + 1);
        harray_ele ele;
        harray_geti(p, i, &ele);
        ASSERT(ele._sint32 == val);
    }
    int val = 10086;
    harray_set(p, 1, &val);
    harray_ele ele;
    harray_geti(p, 1, &ele);
    ASSERT(ele._sint32 == val);

    harray_remove(p, &val);
    ASSERT(harray_get_count(p) == loop_c - 1);

    harray_add(p, -1, &val);
    ASSERT(harray_get_count(p) == loop_c);

    int index = harray_indexOf(p, &val);
    ASSERT(index == loop_c - 1);

    harray_remove_at(p, 1, &ele);
    ASSERT(harray_get_count(p) == loop_c - 1);

    harray_delete(p);
}

void test_array_list(){
    int dt = kType_S32;
    array_list_p p = array_list_new2(8);
    int loop_c = 10;
    for(int i = 0 ; i < loop_c ; i ++){
        void* t = VarArray_new(dt_size(dt), 8);
        array_list_add(p, t);
        void* t2 = array_list_get(p, i);
        ASSERT(t == t2);
    }
    _free_VarArray(array_list_get(p, 1));
    void* newEle = VarArray_new(dt_size(dt), 8);
    array_list_set(p, 1, newEle);
    ASSERT(array_list_get(p, 1) == newEle);
    _free_VarArray(array_list_remove_at(p, 1));
    ASSERT(array_list_size(p) == loop_c - 1);
    array_list_delete2(p, _free_VarArray);
}

void test_var_array(){
    int dt = kType_S32;
    VarArray_p p = VarArray_new(dt_size(dt), 8);
    int loop_c = 10;
    int val2 = -1;
    for(int i = 0 ; i < loop_c ; i ++){
        int val = i;
        VarArray_add(p, i, &val);
        ASSERT(VarArray_valid_data_size(p) == dt_size(dt) * (i + 1));
        VarArray_get(p, i, &val2);
        ASSERT(val2 == val);
    }
    VarArray_set(p, 0, &dt);
    VarArray_get(p, 0, &val2);
    ASSERT(val2 == dt);
    ASSERT(VarArray_indexOf(p, &val2, dt) == 0);
    VarArray_remove_at(p, 2);
    ASSERT(VarArray_valid_data_size(p) == dt_size(dt) * (loop_c - 1));


    VarArray_delete(p, 1);
}
void test_binary_search(){

}
