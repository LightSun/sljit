#include "jit-middle/h7/jit_middle.h"
#include "common/halloc.h"

hjit_obj* hjit_obj_create(hjit_scope* scope, harray_p ps){

    int size = harray_get_count(ps);
    harray_p fs = harray_new(kType_P_STRING, 8);
    harray_p offsets = harray_new(kType_U32, 8);
    uint32 data_size = 0;
    for(int i = 0 ; i < size ; ++i){
        hjit_obj_param* param = (hjit_obj_param*)harray_get_ptr_at(ps ,i);
        hjit_desc_info info;
        if(hjit_scope_get_type_info(scope, param->type_desc, &info) != kState_OK){
            goto FAILED;
        }
        data_size += info.size;
        Utf8String_ref(param->type_desc);
        harray_add2(fs, param->type_desc);
    }
    hjit_obj* obj = ALLOC_T(hjit_obj);
    obj->field_type_descs = fs;
    obj->field_offsets = offsets;
    obj->data = ALLOC(data_size);
    return obj;
FAILED:
    harray_delete(fs);
    harray_delete(offsets);
    return NULL;
}
