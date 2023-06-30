#include "h7/var_array.h"
#include "h7/h_list.h"

#include "h7/utf8_string.h"
#include "h7/h_array.h"
#include "h7/array_map.h"
#include "h7/h_field.h"
#include "h7/h_method.h"
#include "h7/h_object.h"

static void test_var_array();
static void test_array_list();

static void _free_VarArray(void* ele){
    VarArray_p p = (VarArray_p)ele;
    VarArray_delete(p, 1);
}

void test_jit_middle_all(){
    test_var_array();
    test_array_list();
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
    _free_VarArray(array_list_remove_by_index(p, 1));
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
    VarArray_remove(p, 0);
    ASSERT(VarArray_valid_data_size(p) == dt_size(dt) * (loop_c - 1));

    VarArray_delete(p, 1);
}
