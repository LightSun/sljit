#ifndef DTYPES_H
#define DTYPES_H

#include "h7/common/c_common.h"
#include "h7/common/halloc.h"
#include <memory.h>
#include <string.h>

#define kState_FAILED 0
#define kState_OK 1
#define kState_NEXT -1
#define DEFAULT_HASH_SEED 0
#define IOBJ_NAME_MAX_SIZE 28
#define DEFAULT_LOAD_FACTOR 0.75f

#define DT_OFFSET_NONE -1

static inline char* hstrdup(const char* str){
    char* data = ALLOC(strlen(str) + 1);
    strcpy(data, str);
    return data;
}

union htype_value{
    sint8 _sint8;
    uint8 _uint8;
    sint16 _sint16;
    uint16 _uint16;
    sint32 _sint32;
    uint32 _uint32;
    sint64 _sint64;
    uint64 _uint64;
    float _float;
    double _double;
    void* _extra;  //can be any object
};

enum DT{
    kType_NONE = -1,
    kType_VOID,   //only used for func-def
    kType_S8 = 1,
    kType_U8,
    kType_S16,
    kType_U16,
    kType_S32,
    kType_U32,
    kType_S64,
    kType_U64,
    kType_F32,
    kType_F64,
    kType_P_ARRAY,
    kType_P_MAP,
    kType_P_STRING,
    kType_P_OBJECT,
    kType_P_FUNC,
    kType_P_FIELD,
};

typedef struct IObject IObject;
typedef struct hstring hstring;
typedef struct IClass IClass;
typedef struct harray harray;
typedef struct hscope hscope;
typedef void* IObjPtr;

extern hscope* hscope_get_current();
extern IClass* hscope_get_type(hscope* cur, const char* type);
extern void hscope_put_type(hscope* cur, const char* type, IClass* cls);

struct IClass{
    volatile int ref;
    char* type_desc;
   // harray* super_interfaces;
    IObjPtr (*Func_copy)(IObjPtr src, IObjPtr dst);
    int (*Func_equals)(IObjPtr src, IObjPtr dst);
    uint32 (*Func_hash)(IObjPtr src, uint32 seed);
    void (*Func_dump)(IObjPtr src, hstring*);
    void (*Func_ref)(IObjPtr src, int c);
};

struct IObject{
    volatile int ref;
    uint32 flags;
    struct IObject* super;
    IClass* class_info;
};

IClass* IClass_new(const char* type_desc);

void* dtype_obj_cpy(void* ud, void* ele);
int dtype_obj_equals(void* ud, void* ele1, void* ele2);
uint32 dtype_obj_hash(void* ud, void* ele, uint32 seed);
void dtype_obj_ref(void* ud, void* ele, int ref);
void dtype_obj_dump(void* ud, void* ele, hstring*);

void dtype_obj_log(void* ud, void* ele);

int dtype_base_eq(int dt,void* p1, void* p2);
void dtype_base_dump(int dt,void* p1, hstring* out);

static inline void dtype_obj_delete(void* ud, void* ele){
    dtype_obj_ref(ud, ele, -1);
}
//base info of object is eq or not.
int IObject_eqauls_base(void* p1, void* p2);
//
typedef int (*dt_func_eq)(void* ud, void* e1, void* e2);
typedef void* (*dt_func_cpy)(void* ud, void* e1);
typedef uint32 (*dt_func_hash)(void* ud, void* ele, uint32 seed);
typedef void (*dt_func_delete)(void* ud, void* ele);
typedef void (*dt_func_dump)(void* ud, void* ele, hstring*);

static inline void IObject_free(void* arr){
    IObject* obj = (IObject*)arr;
    FREE(arr);
}
static inline void IObject_copy_base_unsafe(int type, void* src1, void* dst1){
    IObject* src = (IObject*)src1;
    IObject* dst = (IObject*)dst1;
    dst->flags = src->flags;
    dst->class_info = src->class_info;
    //dst->super
    dtype_obj_ref(&type, src->super, 1);
    dst->super = src->super;
}

static inline void IObject_copy_base(int type, void* src1, void* dst1){
    IObject* src = (IObject*)src1;
    IObject* dst = (IObject*)dst1;
    dst->flags = src->flags;
    dst->class_info = src->class_info;
    //dst->super
    dtype_obj_ref(&type, dst->super, -1);
    dtype_obj_ref(&type, src->super, 1);
    dst->super = src->super;
}

#define FREE_OBJ(arr) IObject_free(arr)

#define DEF_IOBJ_INIT_CHILD(T, type_desc)\
static inline void __##T##_init(T* arr){\
    arr->baseObj.ref = 1;\
    arr->baseObj.super = NULL;\
    arr->baseObj.flags = 0;\
    IObject* obj = (IObject*)arr;\
    hscope* scope = hscope_get_current();\
    IClass* cls = hscope_get_type(scope, type_desc);\
    if(cls == NULL){\
        cls = IClass_new(type_desc);\
        hscope_put_type(scope, type_desc, cls);\
        cls->Func_copy = Func_copy0;\
        cls->Func_dump = Func_dump0;\
        cls->Func_equals = Func_equals0;\
        cls->Func_hash = Func_hash0;\
        cls->Func_ref = Func_ref0;\
    }\
    obj->class_info = cls;\
}

#define DEF_IOBJ_CHILD_FUNCS(t)\
static inline t* t##_copy(t* src){\
    IObject* obj = (IObject*)src;\
    return (t*)obj->class_info->Func_copy(src, NULL);\
}\
static inline void t##_delete(t* arr){\
    IObject* obj = (IObject*)arr;\
    obj->class_info->Func_ref(arr, -1);\
}\
static inline int t##_equals(t* arr, t* arr2){\
    int ret = IObject_eqauls_base(arr, arr2);\
    if(ret != kState_NEXT){\
        return ret;\
    }\
    IObject* obj = (IObject*)arr;\
    return obj->class_info->Func_equals(arr, arr2);\
}\
static inline uint32 t##_hash(t* arr, uint32 seed){\
    IObject* obj = (IObject*)arr;\
    return obj->class_info->Func_hash(arr, seed);\
}\
static inline void t##_dump(t* arr, struct hstring* hs){\
    IObject* obj = (IObject*)arr;\
    obj->class_info->Func_dump(arr, hs);\
}\
static inline void t##_ref(t* arr){\
    IObject* obj = (IObject*)arr;\
    obj->class_info->Func_ref(arr, 1);\
}\
static inline void t##_unref(t* arr){\
    IObject* obj = (IObject*)arr;\
    obj->class_info->Func_ref(arr, -1);\
}

//-----------------------------------
static inline uint32 dt_size(int dt){
    switch (dt) {
    case kType_S8:
    case kType_U8:
        return sizeof (char);
    case kType_S16:
    case kType_U16:
        return sizeof (short);
    case kType_S32:
    case kType_U32:
        return sizeof (int);
    case kType_S64:
    case kType_U64:
        return sizeof (long long);

    case kType_F32:
        return sizeof (float);
    case kType_F64:
        return sizeof (double);

    case kType_P_ARRAY:
    case kType_P_MAP:
    case kType_P_STRING:
    case kType_P_OBJECT:
    case kType_P_FUNC:
    case kType_P_FIELD:
        return sizeof(void*);
    }
    return 0;
}

static inline uint32 dt_is_base(int dt){
    switch (dt) {
        case kType_S8:
        case kType_U8:
        case kType_S16:
        case kType_U16:
        case kType_S32:
        case kType_U32:
        case kType_S64:
        case kType_U64:
        case kType_F32:
        case kType_F64:
            return 1;
    }
    return 0;
}

static inline uint32 dt_is_float(int dt){
    return dt == kType_F32 || dt == kType_F64;
}

static inline const char* dt2str(int dt){
    switch (dt) {
    case kType_S8:
        return "int8";
    case kType_U8:
        return "uint8";
    case kType_S16:
        return "int16";
    case kType_U16:
        return "uint16";
    case kType_S32:
        return "int32";
    case kType_U32:
        return "uint32";
    case kType_S64:
        return "int64";
    case kType_U64:
        return "uint64";

    case kType_F32:
        return "float";
    case kType_F64:
        return "double";

    case kType_P_ARRAY:
        return "$array";
    case kType_P_MAP:
        return "$map";
    case kType_P_STRING:
        return "$string";
    case kType_P_OBJECT:
        return "$object";
    case kType_P_FUNC:
        return "$function";
    case kType_P_FIELD:
        return "$field";

    case kType_VOID:
        return "$void";
    }
    return "";
}

static inline int str2dt(const char* str){
    if(strcmp(str, "int8")) return kType_S8;
    if(strcmp(str, "uint8")) return kType_U8;
    if(strcmp(str, "int16")) return kType_S16;
    if(strcmp(str, "uint16")) return kType_U16;
    if(strcmp(str, "int32")) return kType_S32;
    if(strcmp(str, "uint32")) return kType_U32;
    if(strcmp(str, "int64")) return kType_S64;
    if(strcmp(str, "uint64")) return kType_U64;
    if(strcmp(str, "float")) return kType_F32;
    if(strcmp(str, "double")) return kType_F64;

    if(strcmp(str, "$array")) return kType_P_ARRAY;
    if(strcmp(str, "$map")) return kType_P_MAP;
    if(strcmp(str, "$string")) return kType_P_STRING;
    if(strcmp(str, "$function")) return kType_P_FUNC;
    if(strcmp(str, "$field")) return kType_P_FIELD;
    if(strcmp(str, "$void")) return kType_VOID;

    return kType_NONE;
}

static inline int dt_is_pointer(int dt){
    switch (dt) {
    case kType_P_ARRAY:
    case kType_P_MAP:
    case kType_P_STRING:
    case kType_P_OBJECT:
    case kType_P_FUNC:
    case kType_P_FIELD:
        return 1;
    }
    return 0;
}

#define DEF_DT_BASE_SWITCH(macro, ffi_t)\
switch (ffi_t) {\
macro(kType_S8, sint8)\
macro(kType_U8, uint8)\
macro(kType_S16, sint16)\
macro(kType_U16, uint16)\
macro(kType_S32, sint32)\
macro(kType_U32, uint32)\
macro(kType_S64, sint64)\
macro(kType_U64, uint64)\
macro(kType_F32, float)\
macro(kType_F64, double)\
}

#define DEF_DT_BASE_SWITCH_FORMAT(macro, ffi_t)\
switch (ffi_t) {\
macro(kType_S8, sint8, "%d")\
macro(kType_U8, uint8, "%d")\
macro(kType_S16, sint16, "%d")\
macro(kType_U16, uint16, "%d")\
macro(kType_S32, sint32, "%d")\
macro(kType_U32, uint32, "%u")\
macro(kType_S64, sint64, "%lld")\
macro(kType_U64, uint64, "%llu")\
macro(kType_F32, float, "%g")\
macro(kType_F64, double, "%g")\
}

#define DEF_DT_ALL_SWITCH(macro, ffi_t)\
switch (ffi_t) {\
macro(kType_S8, "sint8")\
macro(kType_U8, "uint8")\
macro(kType_S16, "sint16")\
macro(kType_U16, "uint16")\
macro(kType_S32, "sint32")\
macro(kType_U32, "uint32")\
macro(kType_S64, "sint64")\
macro(kType_U64, "uint64")\
macro(kType_F32, "float")\
macro(kType_F64, "double")\
macro(kType_P_ARRAY, "<array>")\
macro(kType_P_MAP, "<map>")\
macro(kType_P_STRING, "<string>")\
macro(kType_P_OBJECT, "<object>")\
macro(kType_P_FUNC, "<function>")\
macro(kType_P_FIELD, "<field>")\
macro(kType_VOID, "<void>")\
}

#endif // DTYPES_H
