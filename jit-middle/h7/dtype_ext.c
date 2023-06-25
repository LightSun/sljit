
#include "h7/common/dtypes.h"

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
