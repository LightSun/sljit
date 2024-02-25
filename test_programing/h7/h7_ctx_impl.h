#pragma once

#include "h7_ctx.h"
#include "h7_alloc.h"

#include "h7/common/common.h"

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

ClassInfo::ClassInfo(TypeInfo* arr){
    if(arr){
        arrayDesc = std::make_unique<ArrayClassDesc>();
        arrayDesc->arrayDesc = *arr->arrayDesc;
        arrayDesc->type = arr->type;
    }else{
        fieldMap = std::make_unique<HashMap<String, FieldInfo>>();
    }
}

}
