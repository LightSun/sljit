#pragma once

#include "h7/h7_ctx.h"
#include "h7/ClassScope.h"

namespace h7 {

enum ObjectFlag{
    kFlag_ARRAY = 0x0001,
};

//TODO auto ref/unref for parent-child .
typedef struct Object{
    MemoryBlock block;       ///data block
    ClassInfo* clsInfo;
    Object* parent {nullptr};
    void* offsets {nullptr}; ///just hold. not create in here
    volatile int refCount {1};
    int flags {0};

    void* getDataAddress()const{return block.data;}

    void ref();
    void unref();

} *ObjectPtr;

class ObjectDelegate{
public:
    ObjectDelegate(ObjectPtr ptr):m_ptr(ptr){}

    void* getDataAddress()const{return m_ptr->block.data;}

    FieldInfo* getFieldInfo(CString fn){
        return m_ptr->clsInfo->scope->getFieldInfo(m_ptr->clsInfo->name, fn);
    }
    FieldInfo* getFieldInfo(UInt key){
        return m_ptr->clsInfo->scope->getFieldInfo(m_ptr->clsInfo->name, key);
    }
    int getFieldOffset(CString fn){
        return m_ptr->clsInfo->scope->getFieldOffset(m_ptr->clsInfo->name, fn);
    }
    int getFieldOffset(UInt key){
        return m_ptr->clsInfo->scope->getFieldOffset(m_ptr->clsInfo->name, key);
    }
    void* getFieldAddress(CString fn){
        int offset = getFieldOffset(fn);
        if(offset >= 0){
            return (UChar*)getDataAddress() + offset;
        }
        return nullptr;
    }
    void* getFieldAddress(UInt offset){
        return (UChar*)getDataAddress() + offset;
    }

    bool getField(CString fieldName, Value* out);
    bool setField(CString fieldName, CValue val);

private:
    ObjectPtr m_ptr;
};

class ArrayDelegate{
public:
    ArrayDelegate(ObjectPtr ptr):m_ptr(ptr){}
    ~ArrayDelegate();

    ArrayDelegate& operator=(ArrayDelegate& t){this->m_ptr = t.m_ptr; return *this;}
    ArrayDelegate& operator=(const ArrayDelegate& t){this->m_ptr = t.m_ptr; return *this;}

    UInt getBaseType()const{return m_ptr->clsInfo->arrayDesc->type;}
    UInt getLength()const {return m_ptr->clsInfo->arrayDesc->arrayDesc[0];}
    UInt getTotalLength() const{
        UInt total = 1;
        for(auto& e : m_ptr->clsInfo->arrayDesc->arrayDesc){
            total *= e;
        }
        return total;
    }
    bool elementIsArray()const{
        return m_ptr->clsInfo->arrayDesc->arrayDesc.size() > 1;
    }
    bool elementIsPrimitive()const {return !elementIsArray() && TypeInfo(m_ptr->clsInfo->arrayDesc->type).isPrimitiveType();}
    void* getElementAddr(int index);

    ObjectPtr getElementAsObject(int index);
    ArrayDelegate getElementAsArray(int index);
    bool getElementAsBool(int index){
        auto addr = getElementAddr(index);
        return *((bool*)addr);
    }

private:
    ObjectPtr m_ptr;
};

class Classer
{
public:
    Classer(ClassScope* scope){m_scope = scope;}
    ~Classer(){};

    ClassScope* getScope(){return m_scope;}
    ClassHandle defineClass(CString name, CListTypeInfo fieldTypes,CListString fns);
    RawStringHandle defineRawString(CString name, CString initVal);
    ObjectPtr create(ClassHandle handle, ObjectPtr parent);

private:
    Classer& operator=(const Classer&) = delete ;
    Classer& operator=(Classer&) = delete;
    Classer(const Classer&) = delete;
    Classer(Classer&) = delete;

private:
    ClassScope* m_scope;
};
}
//
extern h7::UInt gObject_get_element_size(h7::ObjectPtr ptr, int arrLevel);
