#ifndef H_MEMBER_H
#define H_MEMBER_H

#include "h7/common/dtypes.h"

typedef struct hmember{
    IObject baseObj;
    int dt;
    int onlyType;
    union htype_value value;
} hmember, *hmember_p;

DEF_IOBJ_CHILD_FUNCS(hmember)

hmember_p hmember_new(int dt);

static inline void hmember_as_onlyType(hmember_p p){
    p->onlyType = 1;
}
static inline int hmember_set_value(hmember_p p,union htype_value* val_ptr){
    p->value = *val_ptr;
    return kState_OK;
}
static inline int hmember_get_value(hmember_p p,union htype_value* val_ptr){
    *val_ptr = p->value;
    return kState_OK;
}

#endif // HMEMBER_H
