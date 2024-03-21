#pragma once

#include "h7_ctx.h"
#include "h7_alloc.h"
#include "h7/common/common.h"
#include <memory.h>

#define __DATA_STACK_FUNC_SET(T)\
void setAt(int index, T c){\
    T* data = (T*)(m_data + index * sizeof (void*));\
    data[0] = c;\
}

namespace h7 {

class DataStack
{
public:
    DataStack(UInt c):m_count(c){
        if(c > 0){
            m_data = (char*)H7_NEW(c * sizeof(void*));
            memset(m_data, 0, c * sizeof(void*));
        }else{
            m_data = nullptr;
        }
    }
    ~DataStack(){
        if(m_data){
            H7_DELETE(m_data);
            m_data = nullptr;
        }
    }
    void addCount(UInt c){
        setCount(m_count + c);
    }
    void setCount(UInt c){
        if(m_data){
            H7_DELETE(m_data);
            m_data = nullptr;
        }
        m_data = (char*)H7_NEW(c * sizeof(void*));
        memset(m_data, 0, c * sizeof(void*));
    }
    UInt getCount(){return m_count;}
    void* getDataPtr(){return m_data;}
    __DATA_STACK_FUNC_SET(i8);
    __DATA_STACK_FUNC_SET(u8);
    __DATA_STACK_FUNC_SET(i16);
    __DATA_STACK_FUNC_SET(u16);
    __DATA_STACK_FUNC_SET(i32);
    __DATA_STACK_FUNC_SET(u32);
    __DATA_STACK_FUNC_SET(i64);
    __DATA_STACK_FUNC_SET(u64);
    __DATA_STACK_FUNC_SET(void*);

    template<typename T>
    T getAt(int index){
        T* data = (T*)(m_data + index * sizeof (void*));
        return data[0];
    }

private:
    __DISABLE_COPY_MOVE(DataStack);
    UInt m_count;
    char* m_data;
};

}

