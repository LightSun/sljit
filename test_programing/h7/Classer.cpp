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

static inline UInt _getSubElementSizeInBytes(const List<UInt>& shape, int arrLevel, int minEleSize){
    UInt eleC = 1;
    for(size_t i = arrLevel + 1 ; i < shape.size() ; ++i){
        eleC *= shape[i];
    }
    return eleC * minEleSize;
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
            ArrayDelegate arrDel(obj);
            int totalLen = arrDel.getTotalLength();
            for(int i = 0 ; i < totalLen ; ++i){
                auto objEle = create((ClassHandle)clsInfo, obj);
                arrDel.setElementAsObjectForTotal(i, objEle);
            }
        }
        obj->offsets = ci->arrayDesc->shape.data();
    }
    return obj;
}

h7::Long gObject_get_element_size(h7::ObjectPtr ptr, int arrLevel){
    return ptr->clsInfo->arrayDesc->elementSize(arrLevel);
}
