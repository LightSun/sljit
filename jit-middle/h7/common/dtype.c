
#include "h7/common/dtypes.h"
#include "h7/h_atomic.h"
#include "h7/hash.h"

void* dtype_obj_cpy(void* ud, void* ele){
    int dt = *(int*)(ud);
    switch (dt) {
    case kType_P_FUNC:
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
    int dt = *(int*)(ud);
    switch (dt) {
    case kType_P_FUNC:
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

int dtype_obj_equals(void* ud, void* ele1, void* ele2){
    int dt = *(int*)(ud);
    switch (dt) {
    case kType_P_FUNC:
    case kType_P_STRING:
    case kType_P_ARRAY:
    case kType_P_OBJECT:
    case kType_P_MAP:{
        IObject* obj = (IObject*)ele1;
        IObject* obj2 = (IObject*)ele2;
        if(strcmp(obj->name, obj2->name)!= 0){
            return kState_FAILED;
        }
        return obj->Func_equals(ele1, ele2);
    }
    }
    return kState_FAILED;
}

void dtype_obj_delete(void* ud, void* ele){
    int dt = *(int*)(ud);
    switch (dt) {
    case kType_P_FUNC:
    case kType_P_STRING:
    case kType_P_ARRAY:
    case kType_P_OBJECT:
    case kType_P_MAP:{
        IObject* obj = (IObject*)ele;
        return obj->Func_ref(ele, -1);
    }
    }
}

void dtype_obj_dump(void* ud, void* ele, hstring* hs){
    int dt = *(int*)(ud);
    switch (dt) {
    case kType_P_FUNC:
    case kType_P_STRING:
    case kType_P_ARRAY:
    case kType_P_OBJECT:
    case kType_P_MAP:{
        IObject* obj = (IObject*)ele;
        obj->Func_dump(ele, hs);
        break;
    }
    }
}

void dtype_obj_ref(void* ele, int c){
    IObject* obj = (IObject*)ele;
    obj->Func_ref(ele, c);
}
