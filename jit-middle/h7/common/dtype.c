
#include <string.h>
#include "h7/common/dtypes.h"
#include "h7/h_atomic.h"
#include "h7/hash.h"
#include "h7/h_string.h"

int IObject_eqauls_base(void* p1, void* p2){
    if(p1 == NULL){
        if(p2 == NULL){
            return kState_OK;
        }else{
            return kState_FAILED;
        }
    }else{
        if(p2 == NULL){
            return kState_FAILED;
        }
        if(p1 == p2){
            return kState_OK;
        }
        IObject* obj1 = (IObject*)p1;
        IObject* obj2 = (IObject*)p2;
        if(strcmp(obj1->class_info->type_desc, obj2->class_info->type_desc) != 0){
            return kState_FAILED;
        }
    }
    return kState_NEXT;
}

void* dtype_obj_cpy(void* ud, void* ele){
    int dt = *(int*)(ud);
    switch (dt) {
    case kType_P_FUNC:
    case kType_P_FIELD:
    case kType_P_STRING:
    case kType_P_ARRAY:
    case kType_P_OBJECT:
    case kType_P_MAP:{
        IObject* obj = (IObject*)ele;
        return obj->class_info->Func_copy(ele, NULL);
    }
    }
    return NULL;
}

uint32 dtype_obj_hash(void* ud, void* ele, uint32 seed){
    int dt = *(int*)(ud);
    switch (dt) {
    case kType_P_FUNC:
    case kType_P_FIELD:
    case kType_P_STRING:
    case kType_P_ARRAY:
    case kType_P_OBJECT:
    case kType_P_MAP:{
        IObject* obj = (IObject*)ele;
        return obj->class_info->Func_hash(ele, seed);
    }
    }
    return seed;
}

int dtype_obj_equals(void* ud, void* ele1, void* ele2){
    int dt = *(int*)(ud);
    switch (dt) {
    case kType_P_FUNC:
    case kType_P_FIELD:
    case kType_P_STRING:
    case kType_P_ARRAY:
    case kType_P_OBJECT:
    case kType_P_MAP:{
        IObject* obj = (IObject*)ele1;
        IObject* obj2 = (IObject*)ele2;
        if(strcmp(obj->class_info->type_desc, obj2->class_info->type_desc)!= 0){
            return kState_FAILED;
        }
        return obj->class_info->Func_equals(ele1, ele2);
    }
    }
    return kState_FAILED;
}

void dtype_obj_ref(void* ud, void* ele, int c){
    if(!ele){
        return;
    }
    int dt = *(int*)(ud);
    switch (dt) {
    case kType_P_FUNC:
    case kType_P_FIELD:
    case kType_P_STRING:
    case kType_P_ARRAY:
    case kType_P_OBJECT:
    case kType_P_MAP:{
        IObject* obj = (IObject*)ele;
        return obj->class_info->Func_ref(ele, c);
    }
    }
}

void dtype_obj_dump(void* ud, void* ele, hstring* hs){
    int dt = *(int*)(ud);
    switch (dt) {
    case kType_P_FUNC:
    case kType_P_FIELD:
    case kType_P_STRING:
    case kType_P_ARRAY:
    case kType_P_OBJECT:
    case kType_P_MAP:{
        IObject* obj = (IObject*)ele;
        obj->class_info->Func_dump(ele, hs);
        break;
    }
    }
}

void dtype_obj_log(void* ud, void* ele){
    hstring* hs = hstring_new();
    dtype_obj_dump(ud, ele, hs);
    hstring_log_and_delete(hs);
}
