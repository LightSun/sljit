#pragma once

#include <unordered_map>
#include "h7/h7_ctx.h"
#include "h7/common/common.h"

#define _RETAIN_LS_SIZE 80 //10 params.

namespace h7 {
class LocalStackManager{
public:
    struct Info{
        UInt idx;
        UInt offset;
        UInt size;
    };
    using InfoMap = std::unordered_map<int, Info>;

    LocalStackManager(UInt local_size):m_maxSize(local_size){}

    static std::unique_ptr<LocalStackManager> New(UInt local_size){
        return std::make_unique<LocalStackManager>(local_size);
    }

    bool allocByType(UInt type){
        H7_ASSERT(type != kType_NONE);
        TypeInfo ti(type);
        auto size = ti.virtualSize();
        if(m_nextOffset + size > m_maxSize){
            return false;
        }
        m_map[type] = _newInfo(size);
        m_nextIdx ++;
        m_nextOffset += size;
        return true;
    }
    int getCount(){
        return m_map.size();
    }
    Info* getAt(UInt idx){
        if(idx >= m_nextIdx){
            return nullptr;
        }
        return &m_map[idx];
    }
    UInt getMaxSize()const{
        return m_maxSize;
    }
    UInt getCurrentSize(){
        return m_nextOffset;
    }
    int getCurrentIdx(){
        return m_nextIdx - 1;
    }
    UInt checkCurrentIdx(){
        H7_ASSERT(m_nextIdx > 0);
        return m_nextIdx - 1;
    }

private:
    Info _newInfo(UInt size){
        Info info;
        info.idx = m_nextIdx;
        info.offset = m_nextOffset;
        info.size = size;
        return info;
    }

private:
    UInt m_maxSize;
    UInt m_nextIdx {0};
    UInt m_nextOffset {0};
    InfoMap m_map;
};

}


