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

}
