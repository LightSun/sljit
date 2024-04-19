#include "Classer.h"
#include <stdarg.h>

#include "h7_alloc.h"
#include "h7_locks.h"
#include "h7_ctx_impl.h"
#include "h7/utils/h_atomic.h"
#include "h7/ArrayDelegate.h"

using namespace h7;

Object::~Object(){
    if(clsInfo->isArray() && clsInfo){
        H7_DELETE(clsInfo);
        clsInfo = nullptr;
    }
}
void Object::ref(){
    h_atomic_add(&refCount, 1);
}

void Object::unref(){
    if(h_atomic_add(&refCount, -1) == 1){
        H7_DELETE(this);
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

ClassInfo* Classer::defineClass(CString name,CListTypeInfo fieldTypes,CListString fns){
    return m_scope->defineClass(name, fieldTypes, fns);
}
ClassInfo* Classer::defineArray(const TypeInfo& info){
    return m_scope->defineArray(info);
}
RawStringHandle Classer::defineRawString(CString name, CString initVal){
    //m_scope->defineConstString(initVal);
}

ObjectPtr Classer::create(ClassInfo* ci, ObjectPtr parent){
    //ClassInfo* ci = (ClassInfo*)handle;
    Object* obj = H7_NEW_TYPE(Object);
    obj->block = MemoryBlock::makeUnchecked(ci->structSize);
    obj->clsInfo = ci;
    obj->parent = parent;
    if(ci->objDesc){
        obj->offsets = ci->objDesc->offsets.data();
    }else if(ci->isArray()){
        //
    }
    return obj;
}
ObjectPtr Classer::createArray(ClassInfo* ci, ObjectPtr parent){
    //ClassInfo* ci = (ClassInfo*)handle;
    H7_ASSERT(ci->isArray());
    Object* obj = H7_NEW_TYPE(Object);
    obj->block = MemoryBlock::makeUnchecked(ci->structSize);
    obj->clsInfo = ci;
    obj->parent = parent;
    obj->flags = kFlag_ARRAY;
    //
    obj->offsets = ci->arrayDesc->shape.data();
    //allocate elements for non-primitive.
    if(!ci->arrayDesc->baseIsPrimitive()){
        H7_ASSERT(ci->arrayDesc->clsName);
        //H7_ASSERT(ci->scope);
        auto clsInfo = ci->scope->getClassInfo(*ci->arrayDesc->clsName);
        H7_ASSERT(clsInfo);
//        auto& shape = ci->arrayDesc->shape;
//        for(size_t i = 0 ; i < shape.size() ; ++i){
//            auto& si = shape[i];
//            auto subEleSize = ci->arrayDesc->elementSize(i);
//        }
        ArrayDelegate arrDel(obj);
        int totalLen = arrDel.getTotalLength();
        for(int i = 0 ; i < totalLen ; ++i){
            auto objEle = create(clsInfo, obj);
            arrDel.setElementAsObjectForTotal(i, objEle);
        }
    }
    return obj;
}

ObjectPtr Classer::createArray(ObjectPtr parent, CString type, CListUInt shape){
    auto ti = TypeInfo::fromTypeName(type);
    ti.setShape(shape);
    auto cls = m_scope->defineArray(ti, true);
    return createArray(cls, parent);
}

//-----------------------------
h7::Long gObject_get_element_size(h7::ObjectPtr ptr, int arrLevel){
    return ptr->clsInfo->arrayDesc->elementSize(arrLevel);
}
h7::ObjectPtr gClasser_newArray(h7::ObjectPtr parent, const char* type,
                           int n, ...){
    List<UInt> shape;
    {
        va_list ap;
        va_start(ap, n);
        for(int i = 0 ; i < n ; ++i){
            shape.push_back(va_arg(ap, int));
        }
        va_end(ap);
    }
    ClassScope* scope = ClassScope::getCurrent();
    Classer cls(scope);
    return cls.createArray(parent, String(type), shape);
}
h7::ObjectPtr gClasser_newArray2(h7::ObjectPtr parent, const char* type,
                                int n, UInt* _shape){
    List<UInt> shape;
    for(int i = 0 ; i < n ; ++i){
        shape.push_back(_shape[i]);
    }
    ClassScope* scope = ClassScope::getCurrent();
    Classer cls(scope);
    return cls.createArray(parent, String(type), shape);
}
