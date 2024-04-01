#pragma once

#include "h7/h7_ctx.h"
#include <memory.h>

namespace h7 {

struct ConstPool{
    List<int> rstr_offsets;
    List<char> rstrs;
    int rstrDataLen {0};

    UInt defineConstString(CString str){
        UInt nextIdx = (UInt)rstr_offsets.size();
        UInt oldDataSize = (UInt)rstrs.size();
        rstrs.resize(oldDataSize + str.length() + 1);
        memcpy(rstrs.data() + oldDataSize, str.data(), str.length());
        rstrs.data()[oldDataSize + str.length()] = '\0';
        rstr_offsets.push_back(oldDataSize);
        return nextIdx;
    }
};

}
