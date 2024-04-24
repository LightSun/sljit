#pragma once

#include "h7/h7_ctx.h"
#include <memory.h>
#include <unordered_map>
#include "h7/utils/hash.h"

namespace h7 {

struct ConstPool{
    List<int> rstr_offsets;
    List<char> rstrs;
    std::unordered_map<uint64,String> nonConstStrs;

    char* defineConstString(CString str,UInt* idx){
        if(idx){
            *idx = (UInt)rstr_offsets.size();
        }
        UInt oldDataSize = (UInt)rstrs.size();
        rstrs.resize(oldDataSize + str.length() + 1);
        memcpy(rstrs.data() + oldDataSize, str.data(), str.length());
        rstrs.data()[oldDataSize + str.length()] = '\0';
        rstr_offsets.push_back(oldDataSize);
        return rstrs.data() + oldDataSize;
    }
    char* getConstStringAddress(UInt idx){
        if(idx >= rstr_offsets.size()) return nullptr;
        return rstrs.data() + rstr_offsets[idx];
    }
    const char* getStringAddress(uint64 hash){
        auto it = nonConstStrs.find(hash);
        if(it != nonConstStrs.end()){
            return it->second.data();
        }
        return nullptr;
    }
    const char* defineString(CString str, uint64* hash){
        auto _hash = fasthash64(str.data(), str.length(), 11);
        {
        auto it = nonConstStrs.find(_hash);
        if(hash) *hash = _hash;
        if(it != nonConstStrs.end()){
            return it->second.data();
        }
        }
        auto it = nonConstStrs.insert(std::pair<int,String>(_hash, str));
        return it.first->second.data();
    }

};

}
