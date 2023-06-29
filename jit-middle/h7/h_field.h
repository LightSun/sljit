#ifndef H_MEMBER_H
#define H_MEMBER_H

#include "h7/common/dtypes.h"

typedef struct hfield{
    IObject baseObj;
    int dt;
    sint16 onlyType;
    sint16 flags; //max 65535
    union htype_value value;
    char* name;
} hfield, *hfield_p;

DEF_IOBJ_CHILD_FUNCS(hfield)

hfield_p hfield_new(const char* name, int dt);

static inline void hfield_as_onlyType(hfield_p p){
    p->onlyType = 1;
}
static inline int hfield_set_value(hfield_p p,union htype_value* val_ptr){
    p->value = *val_ptr;
    return kState_OK;
}
static inline int hfield_get_value(hfield_p p,union htype_value* val_ptr){
    *val_ptr = p->value;
    return kState_OK;
}

#endif // HMEMBER_H
