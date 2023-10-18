#ifndef H_OBJECT_H
#define H_OBJECT_H

#include "h7/h_array.h"

typedef struct hscope hscope;
typedef struct hfield hfield;

typedef struct hobject{
    IObject baseObj;
    void* data;
    harray_p fields;  //multi hfield
    harray_p methods; //multi hmethod
}hobject, *hobject_p;

hobject* hobject_new(harray_p fields, harray_p methods);
void* hobject_get_data_ptr(hobject* p, hfield* src);

#define HOBJ_SIZE() sizeof(struct hobject)

#endif // H_OBJECT_H
