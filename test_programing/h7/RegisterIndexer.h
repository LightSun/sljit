#pragma once

#include <unordered_map>
#include "h7/h7_ctx.h"
#include "h7/common/common.h"
#include "h7/common/c_common.h"

#define _RETAIN_LS_SIZE 80 //10 params.

namespace h7 {
class RegisterIndexer{
public:
    RegisterIndexer(UInt local_size):m_maxCount(local_size/8){
    }

    static std::unique_ptr<RegisterIndexer> New(UInt local_size){
        return std::make_unique<RegisterIndexer>(local_size);
    }

    bool allocLocal(){
        m_nextIdx ++;
        return true;
    }
    int getCount(){
        return m_nextIdx;
    }
    UInt getMaxCount()const{
        return m_maxCount;
    }
    UInt getCurrentSize(){
        return m_nextIdx * (UInt)sizeof (void*);
    }
    int getCurrentIdx(){
        return m_nextIdx - 1;
    }
    UInt checkCurrentIdx(){
        H7_ASSERT(m_nextIdx > 0);
        return m_nextIdx - 1;
    }
    //local stack offset
    UInt getLSOffset(UInt index){
        return index * (UInt)sizeof (void*);
    }
    //data stack offset
    UInt getDSOffset(UInt index){
        return index * (UInt)sizeof (void*);
    }
    int save(){
        return m_nextIdx;
    }
    void restore(int id){
        m_nextIdx = id;
    }
private:
    UInt m_maxCount;
    UInt m_nextIdx {0};
};

}


