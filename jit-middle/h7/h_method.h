#ifndef H_METHOD_H
#define H_METHOD_H

#include "h7/common/dtypes.h"
#include "h7/h_array.h"
#include "h7/h_field.h"

typedef struct hmethod{
    IObject baseObj;
    hfield_p ret;
    harray_p params; //hfields
    char* name;
}hmethod, *hmethod_p;

DEF_IOBJ_CHILD_FUNCS(hmethod)

hmethod_p hmethod_new(const char* name, hfield_p ret, harray_p params);

#endif // H_METHOD_H
