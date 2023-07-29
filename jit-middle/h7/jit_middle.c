#include "jit-middle/h7/jit_middle.h"
#include "h_field.h"


hjit_obj* hjit_obj_create(hjit_scope* scope, array_list_p ps){

    harray_p fields = harray_new(kType_P_FIELD, 4);
    int size = array_list_size(ps);
    for(int i = 0 ; i < size ; ++i){
        hjit_obj_param* param = (hjit_obj_param*)array_list_get(ps ,i);
        switch (param->type) {
        case kType_S8:
        case kType_U8:
        case kType_S16:
        case kType_U16:
        case kType_S32:
        case kType_U32:
        case kType_S64:
        case kType_U64:

        case kType_F32:
        case kType_F64:
        case kType_P_STRING:
        {
            hfield_p p = hfield_new(utf8_cstr(param->name), param->type);
            ASSERT(harray_add2(fields, p) == kState_OK);
        }break;

        case kType_P_ARRAY:
        case kType_P_MAP:
        case kType_P_OBJECT:
        case kType_P_FUNC:
        case kType_P_FIELD:
        default:{
            hjit_type_info info;
            ASSERT(hjit_scope_get_type_info(scope, &info) == kState_OK);

        }
        }
    }
    //return hobject_new(fields, NULL);
}
