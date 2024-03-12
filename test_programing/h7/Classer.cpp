#include "Classer.h"
#include "h7_alloc.h"
#include "h7_locks.h"
#include "h7_ctx_impl.h"
#include "h7/utils/h_atomic.h"

using namespace h7;

void Object::ref(){
    h_atomic_add(&refCount, 1);
}

void Object::unref(){
    if(h_atomic_add(&refCount, -1) == 1){
        H7_DELETE_OBJ(this);
    }
}
bool ObjectDelegate::getField(CString fn, Value* out){
    auto info = getFieldInfo(fn);
    if(!info){
        return false;
    }
    int type = info->typeInfo.isPrimitiveType() ? info->typeInfo.type : kType_object;
    gValue_get(getFieldAddress(info->offset), type, out);
    return true;
}
bool ObjectDelegate::setField(CString fn, CValue val){
    auto info = getFieldInfo(fn);
    if(!info){
        return false;
    }
    int type = info->typeInfo.isPrimitiveType() ? info->typeInfo.type : kType_object;
    gValue_set(getFieldAddress(info->offset), type, (Value*)&val);
    return true;
}

ClassHandle Classer::defineClass(CString name,CListTypeInfo fieldTypes,CListString fns){
    return (ClassHandle)m_scope->defineClass(name, fieldTypes, fns);
}
ObjectPtr Classer::create(ClassHandle handle, ObjectPtr parent){
    ClassInfo* ci = (ClassInfo*)handle;
    Object* obj = H7_NEW_TYPE(Object);
    obj->block = MemoryBlock::makeUnchecked(ci->structSize);
    obj->clsInfo = (ClassInfo*)handle;
    obj->parent = parent;
    return obj;
}
