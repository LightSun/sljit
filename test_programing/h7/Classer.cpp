#include "Classer.h"
#include "h7_alloc.h"
#include "h7_locks.h"
#include "h7_ctx_impl.h"

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
    ClassInfo* newClass(CString name,CListTypeInfo fieldTypes){
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

        //ptr_info->fieldNames = std::move(fieldNames);
        //ptr_info->structSize = calStructSize(fieldTypes, &ptr_info->fieldOffsets);
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

#define _ALIGN_SIZE 8
Long Classer::alignStructSize(CListTypeInfo fieldTypes, List<FieldInfo>* fieldInfo){
    //find all 8 size fields.
    //mark head and tail if is 8-size or not.
    //align non-8-size fields by add.
    int size = fieldTypes.size();
    List<int> aligned_idxes;
    for(int i = 0 ; i < size ; ++i){
        if(fieldTypes[i].isAlignSize(_ALIGN_SIZE)){
            aligned_idxes.push_back(i);
        }
    }
    if(aligned_idxes.empty()){

    }else if(aligned_idxes.size() == 1){

    }else{
        bool head_is_aligned = aligned_idxes[0] == 0;
        bool tail_is_aligned = aligned_idxes[aligned_idxes.size()-1]
                == size -1;
        if(!head_is_aligned){
            aligned_idxes.insert(aligned_idxes.begin(), 0);
        }
        if(!tail_is_aligned){
            aligned_idxes.insert(aligned_idxes.end(), size - 1);
        }
        int asize = aligned_idxes.size();
        for(int i = 0 ; i < asize ; ++i){

        }
    }
}
ClassHandle Classer::define(CString name,CListTypeInfo fieldTypes){
    return (ClassHandle)m_impl->newClass(name, fieldTypes);
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
