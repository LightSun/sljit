#ifndef JIT_MIDDLE_H
#define JIT_MIDDLE_H

#include "h_array.h"
#include "array_map.h"
#include "h_list.h"
#include "h_object.h"
#include "utf8_string.h"

typedef struct hjit_scope hjit_scope;
typedef struct hjit_obj_param hjit_obj_param;
typedef struct hjit_type_info hjit_type_info;
typedef struct hjit_object hjit_obj;

struct hjit_scope{
    //const pool: string + obj
    //type info map: type_desc <-> func_to_create_obj
    hjit_scope* parent;
    harray_p const_pool;
    array_map_p type_map;
};

struct hjit_type_info{
    Utf8String* desc;
    uint32 obj_ext_size;     //IObject + self fields, exclude method.
    int dt_val;
    int dt_key;
};

struct hjit_field{
    Utf8String* type_desc;
    uint32 offset;
    uint32 flags;
};

struct hjit_object{
    IObject baseObj;
    harray_p field_type_descs;
    void* data;
};

//List<Student>
struct hjit_obj_param{
    int type;
    Utf8String* type_desc;
    Utf8String* name;
};

int hjit_scope_get_type_info(hjit_scope* scope, hjit_type_info* info);

hjit_obj* hjit_obj_create(hjit_scope* scope, array_list_p ps);

#endif // JIT_MIDDLE_H
