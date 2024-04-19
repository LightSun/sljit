#pragma once

#include "h7/Classer.h"

namespace h7 {

struct ArrayDelegate{

    ObjectPtr m_ptr;
    //UInt childSizeInBytes; //in bytes

    ArrayDelegate(ObjectPtr ptr):m_ptr(ptr){}
    ~ArrayDelegate(){};

    ArrayDelegate& operator=(ArrayDelegate& t){this->m_ptr = t.m_ptr; return *this;}
    ArrayDelegate& operator=(const ArrayDelegate& t){this->m_ptr = t.m_ptr; return *this;}

    TypeInfo getTypeAsBase()const{
        return TypeInfo(m_ptr->clsInfo->arrayDesc->type,
                        m_ptr->clsInfo->arrayDesc->clsName.get());}
    UInt getLength()const {return m_ptr->clsInfo->arrayDesc->shape[0];}
    UInt getTotalLength() const{
        UInt total = 1;
        for(auto& e : m_ptr->clsInfo->arrayDesc->shape){
            total *= e;
        }
        return total;
    }
    bool elementIsArray()const{
        return m_ptr->clsInfo->arrayDesc->shape.size() > 1;
    }
    bool elementIsPrimitive()const {
        return !elementIsArray() && getTypeAsBase().isPrimitiveType();
    }

    void* getElementAddr(int index);

    void setElementAsObjectForTotal(int idx, ObjectPtr e){
         void** dd = (void**)m_ptr->block.data;
         dd[idx] = e;
    }

    ObjectPtr getElementAsObject(int index);
    ArrayDelegate getElementAsArray(int index);
    bool getElementAsBool(int index){
        auto addr = getElementAddr(index);
        return *((bool*)addr);
    }
};


}

