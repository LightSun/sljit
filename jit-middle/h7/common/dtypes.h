#ifndef DTYPES_H
#define DTYPES_H

#include "h7/common/c_common.h"
#include <memory.h>

#define kState_FAILED 0
#define kState_OK 1
#define DEFAULT_HASH_SEED 0
#define IOBJ_NAME_MAX_SIZE 20

enum DT{
    kType_VOID, //only used for func-def
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
};

typedef struct IObject IObject;
typedef struct hstring hstring;
typedef void* IObjPtr;
struct IObject{
    volatile int ref;
    char name[IOBJ_NAME_MAX_SIZE];
    IObjPtr (*Func_copy)(IObjPtr src, IObjPtr dst);
    int (*Func_equals)(IObjPtr src, IObjPtr dst);
    uint32 (*Func_hash)(IObjPtr src, uint32 seed);
    void (*Func_dump)(IObjPtr src, hstring*);
    void (*Func_ref)(IObjPtr src, int c);
};
extern void* dtype_obj_cpy(void* ud, void* ele);
extern uint32 dtype_obj_hash(void* ud, void* ele, uint32 seed);
extern void dtype_obj_delete(void* ud, void* ele);

static inline void IObject_set_name(void* arr, const char* name){
    IObject* obj = (IObject*)arr;
    memset(obj->name, 0, IOBJ_NAME_MAX_SIZE);
    memcpy(obj->name, name, strlen(name));
}

#define DEF_IOBJ_CHILD_FUNCS(t)\
inline t* t##_copy(t* src){\
    IObject* obj = (IObject*)src;\
    return obj->Func_copy(src, NULL);\
}\
inline void t##_delete(t* arr){\
    IObject* obj = (IObject*)arr;\
    obj->Func_ref(arr, -1);\
}\
inline int t##_equals(t* arr, t* arr2){\
    IObject* obj = (IObject*)arr;\
    return obj->Func_equals(arr, arr2);\
}\
inline void t##_dump(t* arr, struct hstring* hs){\
    IObject* obj = (IObject*)arr;\
    obj->Func_dump(arr, hs);\
}\
inline void t##_ref(t* arr){\
    IObject* obj = (IObject*)arr;\
    obj->Func_ref(arr, 1);\
}\
inline void t##_unref(t* arr){\
    IObject* obj = (IObject*)arr;\
    obj->Func_ref(arr, -1);\
}

//-----------------------------------
static inline int dt_size(int dt){
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
        return sizeof(void*);
    }
    return 0;
}

static inline int dt_is_pointer(int dt){
    switch (dt) {
    case kType_P_ARRAY:
    case kType_P_MAP:
    case kType_P_STRING:
    case kType_P_OBJECT:
    case kType_P_FUNC:
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
macro(kType_VOID, "<void>")\
}

#endif // DTYPES_H
