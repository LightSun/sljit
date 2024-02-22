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

TypeInfo::~TypeInfo(){
    if(subTypes){
        delete subTypes;
        subTypes = nullptr;
    }
}
bool TypeInfo::isAlignSize(int expect)const{

    switch (type) {
    case kType_NONE:
        return false;

    case kType_bool:
    case kType_int8:
    case kType_uint8:
        return sizeof(char) == expect;
    case kType_int16:
    case kType_uint16:
        return sizeof(short) == expect;

    case kType_int32:
    case kType_uint32:
        return sizeof(int) == expect;

    case kType_int64:
    case kType_uint64:
        return sizeof(int64_t) == expect;
    }
    return sizeof (void*) == expect;
}

}
