#include "Classer.h"
#include "h7_alloc.h"
#include "h7_locks.h"
#include "h7_ctx_impl.h"
#include "h7/utils/h_atomic.h"

using namespace h7;

Object::~Object(){
    if(clsInfo->isArray() && offsets){
        H7_DELETE(offsets);
        offsets = nullptr;
    }
}
void Object::setArrayOffset(UInt curOffset, UInt childEleSize){
    ArrayInfo* aof = H7_NEW_TYPE(ArrayInfo);
    aof->curOffset = curOffset;
    aof->childEleSize = childEleSize;
    this->offsets = aof;
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

ClassHandle Classer::defineClass(CString name,CListTypeInfo fieldTypes,CListString fns){
    return (ClassHandle)m_scope->defineClass(name, fieldTypes, fns);
}
RawStringHandle Classer::defineRawString(CString name, CString initVal){

}

ObjectPtr Classer::create(ClassHandle handle, ObjectPtr parent){
    ClassInfo* ci = (ClassInfo*)handle;
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
ObjectPtr Classer::createArray(ClassHandle handle, ObjectPtr parent, ArrayOffset* aof){
    ClassInfo* ci = (ClassInfo*)handle;
    H7_ASSERT(ci->isArray());
    Object* obj = H7_NEW_TYPE(Object);
    obj->block = MemoryBlock::makeUnchecked(ci->structSize);
    obj->clsInfo = ci;
    obj->parent = parent;
    obj->flags = kFlag_ARRAY;
    //allocate elements for non-primitive.
    if(!ci->arrayDesc->baseIsPrimitive()){
        H7_ASSERT(ci->arrayDesc->clsName);
        //H7_ASSERT(ci->scope);
        auto clsInfo = ci->scope->getClassInfo(*ci->arrayDesc->clsName);
        H7_ASSERT(clsInfo);
        //set all elements
//            auto& shape = ci->arrayDesc->shape;
//            for(size_t i = 0 ; i < shape.size() ; ++i){

//            }
        //
//            ArrayDelegate arrDel(obj);
//            int totalLen = arrDel.getTotalLength();
//            for(int i = 0 ; i < totalLen ; ++i){
//                auto objEle = create((ClassHandle)clsInfo, obj);
//                arrDel.setElementAsObjectForTotal(i, objEle);
//            }
    }
    if(aof){
        obj->setArrayOffset(aof->curOffset, aof->childEleSize);
    }
    return obj;
}

h7::Long gObject_get_element_size(h7::ObjectPtr ptr, int arrLevel){
    return ptr->clsInfo->arrayDesc->elementSize(arrLevel);
}
