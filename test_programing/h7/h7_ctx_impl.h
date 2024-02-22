#pragma once

#include "h7_ctx.h"
#include "h7_alloc.h"

namespace h7 {

inline MemoryBlock MemoryBlock::makeUnchecked(UInt size){
    MemoryBlock b;
    b.data = H7_NEW(size);
    b.size = size;
    b.allocSize = size;
    return b;
}

TypeInfo TypeInfo::makeSimple(UInt type){
    TypeInfo info; info.type = type; return info;
}
bool TypeInfo::isPrimitiveType()const{
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
bool TypeInfo::isArrayType() const{
    return type == kType_array;
}
bool TypeInfo::isAlignSize(int expect)const{
    return virtualSize() == expect;
}

int TypeInfo::virtualSize()const{
    switch (type) {
    case kType_NONE:
        return 0;

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

}
