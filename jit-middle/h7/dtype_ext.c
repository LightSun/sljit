
#include "h7/common/dtypes.h"
#include "h_atomic.h"
#include "h7/hash.h"

void* dtype_obj_cpy(void* ud, void* ele){
    char dt = *(char*)(ud);
    switch (dt) {
    case kType_P_FUNC:{
        return ele;
    }
    case kType_P_STRING:
    case kType_P_ARRAY:
    case kType_P_OBJECT:
    case kType_P_MAP:{
        IObject* obj = (IObject*)ele;
        return obj->Func_copy(ele, NULL);
    }
    }
    return NULL;
}

uint32 dtype_obj_hash(void* ud, void* ele, uint32 seed){
    char dt = *(char*)(ud);
    switch (dt) {
    case kType_P_FUNC:{
        return fasthash32(ele, sizeof(void*), seed);
    }
    case kType_P_STRING:
    case kType_P_ARRAY:
    case kType_P_OBJECT:
    case kType_P_MAP:{
        IObject* obj = (IObject*)ele;
        return obj->Func_hash(ele, seed);
    }
    }
    return seed;
}

void dtype_obj_delete(void* ud, void* ele){
    char dt = *(char*)(ud);
    switch (dt) {
    case kType_P_FUNC:{
        break;
    }
    case kType_P_STRING:
    case kType_P_ARRAY:
    case kType_P_OBJECT:
    case kType_P_MAP:{
        IObject* obj = (IObject*)ele;
        return obj->Func_ref(ele, -1);
    }
    }
}

void dtype_obj_ref(void* ele, int c){
    IObject* obj = (IObject*)ele;
    obj->Func_ref(ele, c);
}
