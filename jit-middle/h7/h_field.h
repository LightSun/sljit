#ifndef H_MEMBER_H
#define H_MEMBER_H

#include "h7/common/dtypes.h"
#include "h7/utf8_string.h"

typedef struct hfield{
    IObject baseObj;
    uint32 offset; // offset of object
    char* name;
} hfield, *hfield_p;

DEF_IOBJ_CHILD_FUNCS(hfield)

hfield_p hfield_new(const char* name);

#endif // HMEMBER_H
