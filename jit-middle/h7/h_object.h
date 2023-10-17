#ifndef H_OBJECT_H
#define H_OBJECT_H

#include "h7/h_array.h"

typedef struct hscope hscope;
typedef struct hfield hfield;

typedef struct hobject{
    IObject baseObj;
    hscope* scope;
    void* data;
    harray_p fields;  //multi hfield
    harray_p methods; //multi hmethod
}hobject, *hobject_p;

hobject* hobject_new(harray_p fields, harray_p methods);
void hobject_copy_field_data(hobject* p, hfield* src, hfield* dst);

#define HOBJ_SIZE() sizeof(struct hobject)

#endif // H_OBJECT_H
