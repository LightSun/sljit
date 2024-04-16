#pragma once

#include "h7/h7_ctx.h"
#include "h7/ClassScope.h"

namespace h7 {

enum ObjectFlag{
    kFlag_ARRAY = 0x0001,
};

struct ArrayInfo{
    UInt curOffset;    //current offset from the begin of data (in-bytes).
    UInt childEleSize; //child element size in-bytes.
    UInt shape[4] {0,0,0,0};
};

//TODO auto ref/unref for parent-child.
typedef struct Object{
    MemoryBlock block;        ///data block
    ClassInfo* clsInfo;
    List<Object*> children;   ///TODO, the children which will be delete in cur-object's destroy.
    Object* parent {nullptr};
    void* offsets {nullptr};  ///object-offsets, array-ArrayOffset,just hold. not create in here
    volatile int refCount {1};
    int flags {0};

    ~Object();

    void* getDataAddress()const{return block.data;}

    void setArrayOffset(UInt curOffset, UInt childEleSize);

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

class Classer
{
public:
    Classer(ClassScope* scope){m_scope = scope;}
    ~Classer(){};

    ClassScope* getScope(){return m_scope;}
    ClassHandle defineClass(CString name, CListTypeInfo fieldTypes,CListString fns);
    RawStringHandle defineRawString(CString name, CString initVal);
    ObjectPtr create(ClassHandle handle, ObjectPtr parent);

    ObjectPtr createArray(ClassHandle handle, ObjectPtr parent, ArrayOffset* aof);

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
h7::Long gObject_get_element_size(h7::ObjectPtr ptr, int arrLevel);
