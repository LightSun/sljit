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

    using i8 = Char;
    using u8 = UChar;
    using i16 = Short;
    using u16 = UShort;
    using i32 = Int;
    using u32 = UInt;
    using i64 = Long;
    using u64 = ULong;

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
    kType_int16,
    kType_uint16,
    kType_int32,
    kType_uint32,
    kType_int64,
    kType_uint64,
    kType_bool,
    kType_float,
    kType_double,
    kType_object,
    kType_array,
    //kType_map,
    //kType_list,
};

struct MemoryBlock{
    void* data;
    UInt size;
    UInt allocSize;

    static inline MemoryBlock makeUnchecked(UInt size);
};

struct TypeInfo{
    UInt type;
    UInt length {0};

    List<TypeInfo> subTypes;

    static inline TypeInfo makeSimple(UInt type);

    inline bool isPrimitiveType() const;
    inline bool isArrayType() const;
    inline bool isAlignSize(int expect) const;
    inline int virtualSize()const;
};

typedef union Value{
    Char i8;
    UChar u8;
    Short i16;
    UShort u16;
    Int i32;
    UInt u32;
    Long i64;
    ULong u64;
    Float f;
    Double d;
    void* ptr;
}Value;

struct FieldInfo{
    String name;
    TypeInfo typeInfo;
    UInt offset; //offset of object data
};

struct ClassInfo{
    String name; //full name
    UInt structSize;
    HashMap<String, FieldInfo> fieldMap;
};

struct Object{
    volatile int refCount {0};
    ClassHandle clsHandle;
    MemoryBlock block;

    void ref();
    void unref();
};

using ListTypeInfo = List<TypeInfo>;
using CListTypeInfo = const ListTypeInfo&;
using ListString = List<String>;
using CListString = const ListString&;
using CTypeInfo = const TypeInfo&;

using CFieldInfo = const FieldInfo&;
using CValue = const Value&;

void gValue_get(const void* data, UInt type, Value* out);
void gValue_set(const void* data, UInt type, Value* out);

}
