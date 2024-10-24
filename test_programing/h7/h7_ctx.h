#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <list>
#include <array>

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

template<typename T>
    using UPtr = std::unique_ptr<T>;
template<typename T>
    using SPtr = std::unique_ptr<T>;

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
    using ListUI = List<UInt>;
    using ListS = List<String>;

template<typename T>
    using LinkList = std::list<T>;

    using ULongArray3 = std::array<ULong,3>;
    using CULongArray3 = const std::array<ULong,3>&;
    using IntArray3 = std::array<int,3>;
    using CIntArray3 = const IntArray3&;
    using CListUInt = const List<UInt>&;
    using CListInt = const List<Int>&;
    using UIntArray3 = std::array<UInt,3>;
    using UIntArray4 = std::array<UInt,4>;
    using CUIntArray3 = const UIntArray3&;
//-------------------------------------

using ClassHandle = Long;
using RawStringHandle = Long;
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
    kType_raw_str,

    kType_MAX,
};

struct MemoryBlock{
    void* data {nullptr};
    UInt size;
    UInt allocSize; //0 means from exist-buffer.
    static inline MemoryBlock makeUnchecked(UInt size);
    static inline MemoryBlock makeFromBuffer(void* data, UInt size);

    inline ~MemoryBlock();
};

struct TypeInfo{
    UInt type {kType_NONE};
    ///object: class name, array: base name of raw-element.
    std::unique_ptr<String> clsName;

    std::unique_ptr<List<UInt>> shape;        // like p[2][3] -> arrayDesc =[2,3]
    std::unique_ptr<List<TypeInfo>> subDesc; // like map<String,string>

    TypeInfo(){}
    TypeInfo(UInt type):type(type){}
    inline TypeInfo(UInt type, String* clsN);
    inline TypeInfo(const TypeInfo&);
    inline TypeInfo(TypeInfo&);
    inline TypeInfo& operator=(const TypeInfo&);
    inline TypeInfo& operator=(TypeInfo&);

    static inline TypeInfo makeSimple(int type){return TypeInfo(type);}

    template<typename... _Args>
    static std::vector<TypeInfo> makeListSimple(_Args&&... __args);
    static inline TypeInfo fromTypeName(CString name);

    inline static int computePrimitiveType(bool _float, bool _signed, int ret_size);
    inline static int computeAdvanceType(int type1, int type2);

    inline void setShape(CList<UInt> shape);

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
    inline bool is64(){return isAlignSize(sizeof(Long));}
    inline bool isLessInt()const{ return virtualSize() < (int)sizeof(int);}
    /// arrLevel: [0,arrayDesc.size-1]
    inline UInt elementSize(int arrLevel);
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
    UInt index;
    UInt offset; //offset of object data
};

struct ArrayClassDesc{
    UInt type;                       //primitive-type.
    List<UInt> shape;
    std::unique_ptr<String> clsName; //class name for non-primitive

    //only
    inline bool baseIsPrimitive()const;
    /// sub element size, in bytes, arrLevel: [0,arrayDesc.size-1]
    inline UInt elementSize(int arrLevel);
    inline UInt computeOffset(CList<int> idxes);
    inline UInt shapeSize();
    inline void setByTypeInfo(const TypeInfo& ti);
};

struct ObjectClassDesc{
    HashMap<UInt, FieldInfo> fieldMap;
    List<UInt> offsets;

    inline void putField(CString key, const FieldInfo&);
    inline FieldInfo* getField(CString key);
    inline FieldInfo* getField(UInt key);
};

struct ClassScope;

struct ClassInfo{
    ClassScope* scope {nullptr};
    String name;        //full name
    UInt structSize;

    ArrayClassDesc* arrayDesc {nullptr};
    ObjectClassDesc* objDesc {nullptr};

    inline ~ClassInfo();

    //array or normal object
    inline void setUp(const TypeInfo* arr = nullptr);
    inline bool isArray()const {return arrayDesc != nullptr;}
    ///-1 for no found.
    inline int getFieldOffset(CString name);
    inline int getFieldOffset(UInt key);
    inline void putField(CString key, const FieldInfo&);
    inline FieldInfo* getField(CString key);
    inline FieldInfo* getField(UInt key);
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
