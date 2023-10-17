
#include <string.h>
#include "h7/common/dtypes.h"
#include "h7/h_atomic.h"
#include "h7/hash.h"
#include "h7/h_string.h"
#include "h7/numbers.h"

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
    case kType_P_MAP:
    default:
    {
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
    case kType_P_MAP:
    default:{
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
    case kType_P_MAP:
    default:
    {
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
    case kType_P_MAP:
    default:
    {
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
    case kType_P_MAP:
    default:
    {
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

int dtype_base_eq(int dt, void* p1, void* p2){
    if(dt_is_base(dt)){
        switch (dt) {
        case kType_F32:{
            return isFloatEquals2(p1, p2);
        }
        case kType_F64:{
            return isDoubleEquals2(p1, p2);
        }
        }
        return memcmp(p1, p2, dt_size(dt)) == 0;
    }
    return 0;
}
void dtype_base_dump(int dt, void* p1, hstring* out){
    switch (dt) {
    case kType_S8:{
        hstring_appendf(out, "%d", *(char*)p1);
    }break;
    case kType_U8:{
        hstring_appendf(out, "%d", *(uint8*)p1);
    }break;
    case kType_S16:{
        hstring_appendf(out, "%d", *(sint16*)p1);
    }break;
    case kType_U16:{
        hstring_appendf(out, "%d", *(uint16*)p1);
    }break;
    case kType_S32:{
        hstring_appendf(out, "%d", *(sint32*)p1);
    }break;
    case kType_U32:{
        hstring_appendf(out, "%u", *(uint32*)p1);
    }break;
    case kType_S64:{
        hstring_appendf(out, "%lld", *(sint64*)p1);
    }break;
    case kType_U64:{
        hstring_appendf(out, "%llu", *(uint64*)p1);
    }break;
    case kType_F32:{
        hstring_appendf(out, "%g", *(float*)p1);
    }break;
    case kType_F64:{
        hstring_appendf(out, "%g", *(double*)p1);
    }break;
    }
}
