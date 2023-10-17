#ifndef H_MEMBER_H
#define H_MEMBER_H

#include "h7/common/dtypes.h"
#include "h7/utf8_string.h"

typedef struct hobject hobject;

typedef struct hfield{
    IObject baseObj;
    uint32 offset; // offset of object
    //uint32 size;
    char* dt_desc;
    char* name;
    hobject* owner;
} hfield, *hfield_p;

DEF_IOBJ_CHILD_FUNCS(hfield)

hfield_p hfield_new(const char* dt_desc, const char* name);

static inline void hfield_set_owner(hfield_p p, hobject* owner){
    p->owner = owner;
}
static inline void hfield_set_offset(hfield_p p, uint32 offset){
    p->offset = offset;
}

#endif // HMEMBER_H
