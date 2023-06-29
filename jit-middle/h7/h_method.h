#ifndef H_METHOD_H
#define H_METHOD_H

#include "h7/common/dtypes.h"
#include "h7/h_array.h"
#include "h7/h_member.h"

typedef struct hmethod{
    IObject baseObj;
    char* name; //
    hmember_p ret;
    harray_p params;
}hmethod, *hmethod_p;

DEF_IOBJ_CHILD_FUNCS(hmethod)

hmethod_p hmethod_new(hmember_p ret, harray_p params);

#endif // H_METHOD_H
