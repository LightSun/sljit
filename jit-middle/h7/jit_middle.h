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
typedef struct hjit_desc_info hjit_desc_info;

struct hjit_scope{
    //const pool: string + obj
    //type info map: type_desc <-> func_to_create_obj
    hjit_scope* parent;
    array_map_p const_pool; //kv = name, obj
    array_map_p type_map;   //kv = desc, hjit_desc_info
};

struct hjit_Field{
    Utf8String* type_desc;
    uint32 offset;
    uint32 flags;
};

struct hjit_object{
    IObject baseObj;
    harray_p names;
    harray_p fields;    //List<hjit_member>
    void* data;
    uint32 size;
    uint32 flags;
};

struct hjit_desc_info{
    Utf8String* super;         //may be null
    harray_p super_interfaces; //List<Utf8String>
    uint32 size;
};

//List<Student>
struct hjit_obj_param{
    Utf8String* type_desc;
    Utf8String* name;
};

int hjit_scope_get_type_info(hjit_scope* scope, Utf8String* desc,hjit_desc_info* info);

hjit_obj* hjit_obj_create(hjit_scope* scope, harray_p ps);

#endif // JIT_MIDDLE_H
