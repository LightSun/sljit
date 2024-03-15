#pragma once

#include "h7/h7_ctx.h"
#include "h7/h7_alloc.h"
#include "h7/common/common.h"
#include "h7/common/c_common.h"

namespace h7 {

inline MemoryBlock MemoryBlock::makeUnchecked(UInt size){
    MemoryBlock b;
    b.data = H7_NEW(size);
    b.size = size;
    b.allocSize = size;
    return b;
}

TypeInfo::~TypeInfo(){
    if(clsName){
        delete clsName;
        clsName = nullptr;
    }
}
bool TypeInfo::isPrimitiveType()const{
    return baseIsPrimitiveType() && !isArrayType();
}
bool TypeInfo::baseIsPrimitiveType()const{
    if(clsName == nullptr){
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
    }
    return false;
}
bool TypeInfo::hasSubType()const{
    return subDesc && subDesc->size() > 0;
}
bool TypeInfo::isArrayType() const{
    return arrayDesc && arrayDesc->size() > 0;
}
int TypeInfo::getTotalArraySize()const{
    int total = 1;
    for(auto& e: *arrayDesc){
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

String TypeInfo::getTypeDesc()const{
    String str;
    if(clsName){
        str = *clsName;
    }else{
        switch (type) {
        case kType_bool: {str = "B";}break;
        case kType_int8: {str = "I8";}break;
        case kType_uint8: {str = "U8";}break;
        case kType_int16: {str = "I16";}break;
        case kType_uint16: {str = "U16";}break;

        case kType_int32: {str = "I32";}break;
        case kType_uint32: {str = "U32";}break;

        case kType_int64:   {str = "U32";}break;
        case kType_uint64:  {str = "U64";}break;

        case kType_float: {str = "F";}break;
        case kType_double: {str = "D";}break;

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
    if(arrayDesc){
        for(auto& e: *arrayDesc){
            str.append("[");
        }
    }
    return str;
}

ClassInfo::ClassInfo(const TypeInfo* arr){
    if(arr){
        arrayDesc = std::make_unique<ArrayClassDesc>();
        arrayDesc->arrayDesc = *arr->arrayDesc;
        arrayDesc->type = arr->type;
    }else{
        fieldMap = std::make_unique<HashMap<String, FieldInfo>>();
    }
}
int ClassInfo::getFieldOffset(CString name){
    if(fieldMap){
        auto it = fieldMap->find(name);
        if(it != fieldMap->end()){
            return it->second.offset;
        }
    }
    return -1;
}

}
