#pragma once

#include "h7_ctx.h"

namespace h7 {

using ClassHandle = long long;
using ObjectHandle = void*;

enum{
    kType_NONE,
    kType_int8,
    kType_uint8,
    kType_int32,
    kType_uint32,
    kType_int64,
    kType_uint64,
    kType_bool,
    kType_float,
    kType_double,
    kType_ptr,
    kType_array,
    //kType_map,
    //kType_list,
};

struct MemoryBlock{
    void* data;
    UInt allocSize;
    UInt size;
};

struct TypeInfo{
    String name;
    int type;
    unsigned int length {0};
    int subType  {kType_NONE};
    int subType2 {kType_NONE};
};

typedef union Value{
    Char i8;
    UChar u8;
    Int i32;
    UInt u32;
    Long i64;
    ULong ui64;
    Float f;
    double d;
    void* ptr;
}Value;

struct WrapValue{
    TypeInfo type;
    Value val;
};

struct ObjectHeadInfo{
    ClassHandle clsHandle;
    List<UInt> fieldOffsets;
};

struct Object{
    ObjectHeadInfo headInfo;
    MemoryBlock block;
};

class Classer
{
public:
    using ListTI = List<TypeInfo>;
    using CListTI = const ListTI&;
    Classer();

    ClassHandle define(CString name,CListTI types);
    ObjectHandle create(ClassHandle handle);
    WrapValue get(ObjectHandle oh);
};

}

