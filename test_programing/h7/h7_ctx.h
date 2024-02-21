#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>

namespace h7 {
    using Char = char;
    using UChar = unsigned char;
    using Short = short;
    using UShort = unsigned short;
    using Int = int;
    using UInt = unsigned int;
    using Long = long long;
    using ULong = unsigned long long;
    using Float = float;
    using Double = double;

    using String = std::string;
    using CString = const std::string&;
template<typename k, typename v>
    using HashMap = std::unordered_map<k,v>;
template<typename T>
    using List = std::vector<T>;
template<typename T>
    using CList = const std::vector<T>&;
    using ListI = List<int>;
    using ListS = List<String>;

//-------------------------------------

using ClassHandle = Long;
using ObjectHandle = Long;

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
    UInt size;
    UInt allocSize;

    static MemoryBlock makeUnchecked(UInt size);
};

struct TypeInfo{
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
    Double d;
    void* ptr;
}Value;

struct FieldInfo{
    String name;
    TypeInfo typeInfo;
    Value val;
};

struct ClassInfo{
    String name; //full name
    UInt structSize;
    List<UInt> fieldOffsets;
    List<String> fieldNames;
};

struct Object{
    ClassHandle clsHandle;
    MemoryBlock block;
};

using ListTypeInfo = List<TypeInfo>;
using CListTypeInfo = const ListTypeInfo&;
using ListString = List<String>;
using CListString = const ListString&;
using CTypeInfo = const TypeInfo&;

using CFieldInfo = const FieldInfo&;
using CValue = const Value&;

}
