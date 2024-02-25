#include "ClassScope.h"

#include "Classer.h"
#include "h7_alloc.h"
#include "h7_locks.h"
#include "h7_ctx_impl.h"

#include "h7/common/common.h"
#include "h7/utils/h_atomic.h"

using namespace h7;

//in 64. must >= 8
#define _ALIGN_SIZE 8

static ClassScope* s_global_clsScope {nullptr};
static ClassScope* s_cur_clsScope {nullptr};

namespace h7 {
struct _ClassScope_ctx{
    HashMap<String, ClassInfo*> clsMap;
    MutexLock clsLock;


    ~_ClassScope_ctx(){
        MutexLockHolder lck(clsLock);
        auto it = clsMap.begin();
        while(it != clsMap.end()){
            H7_DELETE_OBJ(it->second);
            ++it;
        }
        clsMap.clear();
    }
    ClassInfo* defineClass(ClassScope* scope, CString name,CListTypeInfo fieldTypes, CListString fns){
        ClassInfo* ptr_info;
        {
        MutexLockHolder lck(clsLock);
        auto it = clsMap.find(name);
        if(it != clsMap.end()){
            return 0;
        }
        ptr_info = H7_NEW_OBJ(ClassInfo);
        clsMap[name] = ptr_info;
        }
        ptr_info->name = name;
        ptr_info->scope = scope;
        h7::alignStructSize(fieldTypes, fns, ptr_info);
        return ptr_info;
    }
    ClassInfo* defineArray(ClassScope* scope, const TypeInfo& info){
        auto tyepStr = info.getTypeDesc();
        ClassInfo* ptr_info;
        {
        MutexLockHolder lck(clsLock);
        auto it = clsMap.find(tyepStr);
        if(it != clsMap.end()){
            return 0;
        }
        }
        ptr_info = H7_NEW_OBJ1(ClassInfo, info.arrayDesc.get());
        clsMap[tyepStr] = ptr_info;
        //
        int size = info.virtualSize() * info.getTotalArraySize();
        ptr_info->name = tyepStr;
        ptr_info->scope = scope;
        ptr_info->structSize = size;
        //set sub-arr, like: a[2][3]
        return ptr_info;
    }
    int getFieldOffset(CString clsName, CString fn){
        auto fi = getFieldInfo(clsName, fn);
        return fi ? fi->offset : -1;
    }
    FieldInfo* getFieldInfo(CString clsName, CString fn){
        ClassInfo* info = nullptr;
        {
            MutexLockHolder lck(clsLock);
            auto it = clsMap.find(clsName);
            if(it != clsMap.end()){
                info = it->second;
            }
        }
        if(info != nullptr){
            auto it = info->fieldMap->find(fn);
            if(it != info->fieldMap->end()){
                return &it->second;
            }
        }
        return nullptr;
    }
};
}

ClassScope* ClassScope::createGlobal(){
    auto scope = new ClassScope();
    s_global_clsScope = scope;
    return scope;
}
ClassScope* ClassScope::getGlobal(){
    return s_global_clsScope;
}
ClassScope* ClassScope::getCurrent(){
    return s_cur_clsScope;
}
void ClassScope::enter(ClassScope* t){
    if(s_cur_clsScope){
        s_cur_clsScope->m_children.push_back(t);
    }
    s_cur_clsScope = t;
}
void ClassScope::exit(){
    if(s_cur_clsScope){
        auto back = s_cur_clsScope->m_parent->m_children.back();
        H7_ASSERT_X(back == s_cur_clsScope, "scope state wrong.");
        s_cur_clsScope->m_parent->m_children.pop_back();
    }
    s_cur_clsScope = s_cur_clsScope->m_parent;
}

ClassScope::ClassScope(){
    m_ctx = new _ClassScope_ctx();
}
ClassScope::~ClassScope(){
    if(m_ctx){
        delete m_ctx;
        m_ctx = nullptr;
    }
}

ClassInfo* ClassScope::defineClass(CString name,CListTypeInfo fieldTypes, CListString fns){
    return m_ctx->defineClass(this, name, fieldTypes, fns);
}
ClassInfo* ClassScope::defineArray(const TypeInfo& info){
    return m_ctx->defineArray(this, info);
}
ClassInfo* ClassScope::newArrayClassInfo(const TypeInfo& info){
    //privitive, privitive-arr, obj, obj-arr
    if(info.isArrayType()){
        int size = info.virtualSize() * info.getTotalArraySize();
        auto ptr_info = H7_NEW_OBJ(ClassInfo);
        ptr_info->name = info.getTypeDesc();
        ptr_info->scope = ClassScope::getGlobal();
        ptr_info->structSize = size;
        //set sub-arr, like: a[2][3]
        return ptr_info;
    }
    return nullptr;
}
int ClassScope::getFieldOffset(CString clsName, CString fn){
    return m_ctx->getFieldOffset(clsName, fn);
}
FieldInfo* ClassScope::getFieldInfo(CString clsName, CString fn){
    return m_ctx->getFieldInfo(clsName, fn);
}
