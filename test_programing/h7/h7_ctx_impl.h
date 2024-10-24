#pragma once

#include "h7/h7_ctx.h"
#include "h7/h7_alloc.h"
#include "h7/common/common.h"
#include "h7/common/c_common.h"
#include "h7/utils/hash.h"

#define DEF_HASH_SEED 11

namespace h7 {

MemoryBlock MemoryBlock::makeFromBuffer(void* data, UInt size){
    MemoryBlock b;
    b.data = data;
    b.size = size;
    b.allocSize = 0;
    return b;
}
MemoryBlock MemoryBlock::makeUnchecked(UInt size){
    MemoryBlock b;
    b.data = H7_NEW(size);
    b.size = size;
    b.allocSize = size;
    return b;
}
MemoryBlock::~MemoryBlock(){
    if(allocSize > 0 && data){
        H7_DELETE(data);
        data = nullptr;
    }
}

bool TypeInfo::isPrimitiveType()const{
    return baseIsPrimitiveType() && !isArrayType();
}
bool TypeInfo::baseIsPrimitiveType()const{
    switch (type) {
    case kType_bool:
    case kType_int8:
    case kType_uint8:
    case kType_int16:
    case kType_uint16:

    case kType_int32:
    case kType_uint32:

    case kType_int64:
    case kType_uint64:

    case kType_float:
    case kType_double:
        return true;
    }
    return false;
}
bool TypeInfo::hasSubType()const{
    return subDesc && subDesc->size() > 0;
}
bool TypeInfo::isArrayType() const{
    return shape && shape->size() > 0;
}
int TypeInfo::getTotalArraySize()const{
    int total = 1;
    for(auto& e: *shape){
        total *= e;
    }
    return total;
}
bool TypeInfo::isAlignSize(int expect)const{
    return virtualSize() == expect;
}
int TypeInfo::virtualSize()const{
    switch (type) {
    case kType_NONE:
        return clsName ? sizeof (void*) : 0;

    case kType_bool:
    case kType_int8:
    case kType_uint8:
        return sizeof(char);
    case kType_int16:
    case kType_uint16:
        return sizeof(short);

    case kType_int32:
    case kType_uint32:
        return sizeof(int);

    case kType_int64:
    case kType_uint64:
        return sizeof(int64_t);

    case kType_float:
        return sizeof (float);
    case kType_double:
        return sizeof (double);
    }
    return sizeof (void*);
}

bool TypeInfo::isSigned()const{
    switch (type) {
    case kType_int8:
    case kType_int16:
    case kType_int32:
    case kType_int64:
    case kType_float:
    case kType_double:
        return true;

    case kType_bool:
    case kType_uint8:
    case kType_uint16:
    case kType_uint32:
    case kType_uint64:
        return false;
    }
    H7_ASSERT_X(false, "TypeInfo::isSigned >> should't reach here.");
    return false;
}

bool TypeInfo::isFloatLikeType()const{
    return type == kType_float
            || type == kType_double;
}
int TypeInfo::computePrimitiveType(bool _float, bool _signed, int ret_size){
    if(_float){
        if(ret_size == sizeof(double)){
            return kType_double;
        }else{
            return kType_float;
        }
    }else{
        if(ret_size == sizeof(Long)){
            return _signed ? kType_int64 : kType_uint64;
        }
        else if(ret_size == sizeof(Int)){
            return _signed ? kType_int32 : kType_uint32;
        }
        else if(ret_size == sizeof(Short)){
            return _signed ? kType_int16 : kType_uint16;
        }
        else if(ret_size == sizeof(Char)){
            return _signed ? kType_int8 : kType_uint8;
        }else{
            gError_throwFmt("advancePrimitive: wrong_size = %d.", ret_size);
            return kType_NONE;
        }
    }
}
int TypeInfo::computeAdvanceType(int type1, int type2){
    TypeInfo t_left(type1);
    TypeInfo t_right(type2);
    int leftSize = t_left.virtualSize();
    int rightSize = t_right.virtualSize();
    int maxSize = HMAX(leftSize, rightSize);
    bool hasSigned = t_left.isSigned() || t_right.isSigned();
    bool hasFT = t_left.isFloatLikeType() || t_right.isFloatLikeType();
    return computePrimitiveType(hasFT, hasSigned, maxSize);
}

TypeInfo::TypeInfo(UInt type, String* clsN):type(type){
    if(clsN){
        clsName = std::make_unique<String>();
        *clsName = *clsN;
    }
}
TypeInfo::TypeInfo(const TypeInfo& ti){
    operator=(ti);
}
TypeInfo::TypeInfo(TypeInfo& ti){
    operator=(ti);
}
TypeInfo& TypeInfo::operator=(const TypeInfo& ti){
    this->type = ti.type;
    if(ti.clsName){
        this->clsName = std::make_unique<String>();
        *clsName = *ti.clsName;
    }
    if(ti.shape){
        this->shape = std::make_unique<List<UInt>>();
        *shape = *ti.shape;
    }
    if(ti.subDesc){
        this->subDesc = std::make_unique<List<TypeInfo>>();
        *subDesc = *ti.subDesc;
    }
    return *this;
}
TypeInfo& TypeInfo::operator=(TypeInfo& ti){
    this->type = ti.type;
    if(ti.clsName){
        this->clsName = std::make_unique<String>();
        *clsName = *ti.clsName;
    }
    if(ti.shape){
        this->shape = std::make_unique<List<UInt>>();
        *shape = *ti.shape;
    }
    if(ti.subDesc){
        this->subDesc = std::make_unique<List<TypeInfo>>();
        *subDesc = *ti.subDesc;
    }
    return *this;
}
template<typename... _Args>
std::vector<TypeInfo> TypeInfo::makeListSimple(_Args&&... __args){
    std::vector<int> vec = {std::forward<_Args>(__args)...};
    std::vector<TypeInfo> list;
    for(auto& v : vec){
        list.emplace_back(v);
    }
    return list;
}
TypeInfo TypeInfo::fromTypeName(CString name){
    static std::unordered_map<String, int> type_map = {
        {"b", kType_bool},
        {"i8", kType_int8},
        {"u8", kType_uint8},
        {"i16", kType_int16},
        {"u16", kType_uint16},
        {"i32", kType_int32},
        {"u32", kType_uint32},
        {"i64", kType_int64},
        {"u64", kType_uint64},
        {"f", kType_float},
        {"d", kType_double},
    };
    auto it = type_map.find(name);
    if(it != type_map.end()) return TypeInfo(it->second, nullptr);
    return TypeInfo(kType_NONE, (String*)&name);
}
String TypeInfo::getTypeDesc()const{
    String str;
    if(clsName){
        str = *clsName;
    }else{
        switch (type) {
        case kType_bool: {str = "b";}break;
        case kType_int8: {str = "i8";}break;
        case kType_uint8: {str = "u8";}break;
        case kType_int16: {str = "i16";}break;
        case kType_uint16: {str = "u16";}break;

        case kType_int32: {str = "i32";}break;
        case kType_uint32: {str = "u32";}break;

        case kType_int64:   {str = "i64";}break;
        case kType_uint64:  {str = "u64";}break;

        case kType_float: {str = "f";}break;
        case kType_double: {str = "d";}break;

        case kType_raw_str: {str = "c_str";}break;
        case kType_object: {str = "Object";}break;

        default:
            H7_ASSERT_X(false, "TypeInfo::getTypeDesc >> wrong type");
        }
    }
    if(subDesc){
        str.append("<");
        int size = subDesc->size();
        for(int i = 0 ; i < size ; ++i){
            str.append(subDesc->at(i).getTypeDesc());
            if(i != size - 1){
                str.append(",");
            }
        }
        str.append(">");
    }
    if(shape){
        for(auto& e: *shape){
            str.append("[");
        }
    }
    return str;
}
void TypeInfo::setShape(CList<UInt> s){
    if(!shape){
        shape = std::make_unique<List<UInt>>();
    }
    *shape = s;
}
UInt TypeInfo::elementSize(int arrLevel){
    int size = shape->size();
    if(arrLevel < 0){
        H7_ASSERT_X(arrLevel + size >= 0, "wrong arrLevel");
        arrLevel = size + arrLevel;
    }
    UInt eleC = 1;
    for(int i = arrLevel + 1 ; i < size ; ++i){
        eleC *= (*shape)[i];
    }
    return eleC * virtualSize();
}
//[2][3][5] -> [1] -> 3 * 5 * eleSize.
UInt ArrayClassDesc::elementSize(int arrLevel){
    int size = shape.size();
    if(arrLevel < 0){
        H7_ASSERT_X(arrLevel + size >= 0, "wrong arrLevel");
        arrLevel = size + arrLevel;
    }
    TypeInfo ti(type, clsName ? clsName.get() : nullptr);
    UInt eleC = 1;
    for(int i = arrLevel + 1 ; i < size ; ++i){
        eleC *= shape[i];
    }
    return eleC * ti.virtualSize();
}
UInt ArrayClassDesc::computeOffset(CList<int> idxes){
    int size = idxes.size();
    UInt offset = 0;
    for(int i = 0 ; i < size ; ++i){
        offset += elementSize(i) * idxes[i];
    }
    return offset;
}
UInt ArrayClassDesc::shapeSize(){
    UInt tsize = 1;
    for(size_t i = 0 ; i < shape.size() ; ++i){
        tsize *= shape[i];
    }
    return tsize;
}
bool ArrayClassDesc::baseIsPrimitive()const{
    TypeInfo ti(type, clsName ? clsName.get() : nullptr);
    return ti.baseIsPrimitiveType();
}
void ArrayClassDesc::setByTypeInfo(const TypeInfo& ti){
    type = ti.type;
    shape = *ti.shape;
    clsName = std::make_unique<String>();
    if(ti.clsName){
        *clsName = *ti.clsName;
    }else{
        *clsName = ti.getTypeDesc();
    }
}
//--------------
void ObjectClassDesc::putField(CString key, const FieldInfo& _f){
    FieldInfo& f = (FieldInfo&)_f;
    auto k = fasthash32(key.data(), (UInt)key.length(), DEF_HASH_SEED);
    f.index = fieldMap.size();
    offsets.push_back(f.offset);
    (fieldMap)[k] = std::move(f);
}
FieldInfo* ObjectClassDesc::getField(CString key){
    auto k = fasthash32(key.data(), (UInt)key.length(), DEF_HASH_SEED);
    auto it = fieldMap.find(k);
    return it != fieldMap.end() ? &it->second : nullptr;
}
FieldInfo* ObjectClassDesc::getField(UInt k){
    auto it = fieldMap.find(k);
    return it != fieldMap.end() ? &it->second : nullptr;
}
//--------------
ClassInfo::~ClassInfo(){
    if(arrayDesc){
        delete arrayDesc;
        arrayDesc = nullptr;
    }
    if(objDesc){
        delete objDesc;
        objDesc = nullptr;
    }
}
void ClassInfo::setUp(const TypeInfo* arr){
    arrayDesc = nullptr;
    objDesc = nullptr;
    scope = nullptr;
    if(arr){
        arrayDesc = new ArrayClassDesc();
        arrayDesc->setByTypeInfo(*arr);
    }else{
        objDesc = new ObjectClassDesc();
    }
}
int ClassInfo::getFieldOffset(CString name){
    auto f = getField(name);
    return f ? f->offset :-1;
}
int ClassInfo::getFieldOffset(UInt key){
    auto f = getField(key);
    return f ? f->offset :-1;
}
void ClassInfo::putField(CString key, const FieldInfo& _f){
    if(!objDesc){
         objDesc = new ObjectClassDesc();
    }
    objDesc->putField(key, _f);
}
FieldInfo* ClassInfo::getField(CString key){
    if(objDesc){
        return objDesc->getField(key);
    }
    return nullptr;
}
FieldInfo* ClassInfo::getField(UInt k){
    if(objDesc){
        return objDesc->getField(k);
    }
    return nullptr;
}
}

