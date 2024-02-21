#include "Classer.h"
#include "h7_alloc.h"
#include "h7_locks.h"
#include "h7_pri.h"

using namespace h7;

namespace h7 {

struct _Classer_ctx{
    HashMap<String, ClassInfo*> clsMap;
    MutexLock clsLock;

    ~_Classer_ctx(){
        auto it = clsMap.begin();
        while(it != clsMap.end()){
            H7_DELETE(it->second);
            ++it;
        }
        clsMap.clear();
    }

    static Long calStructSize(CListTypeInfo fieldTypes, List<UInt>* offsets);

    ClassInfo* newClass(CString name,CListTypeInfo fieldTypes,
                  CListString fieldNames){
        ClassInfo* ptr_info;
        {
        MutexLockHolder lck(clsLock);
        auto it = clsMap.find(name);
        if(it != clsMap.end()){
            return 0;
        }
        ptr_info = H7_NEW_TYPE(ClassInfo);
        clsMap[name] = ptr_info;
        }
        ptr_info->name = name;
        ptr_info->fieldNames = std::move(fieldNames);
        ptr_info->structSize = calStructSize(fieldTypes, &ptr_info->fieldOffsets);
        return ptr_info;
    }
};
}

Classer::Classer()
{
    m_impl = new _Classer_ctx();
}
Classer::~Classer()
{
    if(m_impl){
        delete m_impl;
        m_impl = nullptr;
    }
}
ClassHandle Classer::define(CString name,CListTypeInfo fieldTypes,
                            CListString fieldNames){
    return (ClassHandle)m_impl->newClass(name, fieldTypes, fieldNames);
}
ObjectHandle Classer::create(ClassHandle handle){
    ClassInfo* ci = (ClassInfo*)handle;
    Object* obj = H7_NEW_TYPE(Object);
    obj->block = MemoryBlock::makeUnchecked(ci->structSize);
    obj->clsHandle = handle;
    return (ObjectHandle)obj;
}
Value Classer::getField(ObjectHandle oh, CString fieldName){

}
void Classer::setField(ObjectHandle oh, CString fieldName, CValue val){

}
