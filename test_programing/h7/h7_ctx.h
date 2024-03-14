#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <list>

//#define __ENABLE_SAFE_CHECK 1

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
template<typename k, typename v>
    using CHashMap = const std::unordered_map<k,v>&;
template<typename T>
    using List = std::vector<T>;
template<typename T>
    using CList = const std::vector<T>&;
    using ListI = List<int>;
    using ListS = List<String>;

template<typename T>
    using LinkList = std::list<T>;

    using CULongArray3 = const std::array<ULong,3>&;
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

    kType_MAX,
};

struct MemoryBlock{
    void* data;
    UInt size;
    UInt allocSize;

    static inline MemoryBlock makeUnchecked(UInt size);
};

struct TypeInfo{
    UInt type {kType_NONE};
    String* clsName {nullptr};

    std::shared_ptr<List<UInt>> arrayDesc;// like p[2][3] -> arrayDesc =[2,3]
    std::shared_ptr<List<TypeInfo>> subDesc;

    TypeInfo(){}
    TypeInfo(UInt type):type(type){}
    inline ~TypeInfo();

    static inline TypeInfo makeSimple(int type){return TypeInfo(type);}

    inline String getTypeDesc()const;
    inline int getTotalArraySize()const;
    inline bool baseIsPrimitiveType()const;
    inline bool isPrimitiveType() const;
    inline bool hasSubType() const;
    inline bool isArrayType() const;
    inline bool isAlignSize(int expect) const;
    inline int virtualSize()const;
    inline bool isSigned()const;
    inline bool isFloatLikeType()const;
    inline static int computePrimitiveType(bool _float, bool _signed, int ret_size);
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

struct ArrayClassDesc{
    List<UInt> arrayDesc;
    UInt type; //base type
};

struct ClassScope;

struct ClassInfo{
    ClassScope* scope {nullptr};
    String name; //full name
    UInt structSize;

    std::unique_ptr<ArrayClassDesc> arrayDesc;
    std::unique_ptr<HashMap<String, FieldInfo>> fieldMap;

    //array or normal object
    inline ClassInfo(const TypeInfo* arr = nullptr);
    inline bool isArray()const {return arrayDesc != nullptr;}
    ///-1 for no found.
    inline int getFieldOffset(CString name);
};

using ListTypeInfo = List<TypeInfo>;
using CListTypeInfo = const ListTypeInfo&;
using ListString = List<String>;
using CListString = const ListString&;
using CTypeInfo = const TypeInfo&;

using CFieldInfo = const FieldInfo&;
using CValue = const Value&;
//


//--------
extern Long alignStructSize(CListTypeInfo fieldTypes, CListString fns, ClassInfo* out);
extern void gValue_get(const void* data, UInt type, Value* out);
extern void gValue_set(const void* data, UInt type, Value* in);
extern void gValue_rawGet(const void* data, UInt type, void* out);
extern void gValue_rawSet(const void* data, UInt type, void* out);

#define gError_throwFmt(fmt,...)\
    do{\
        char buf[512];\
        snprintf(buf, 512, fmt, ##__VA_ARGS__);\
        __THROW_ERR(String(buf));\
    }while(0);

}
