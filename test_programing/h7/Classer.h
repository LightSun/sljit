#pragma once

#include "h7_ctx.h"

namespace h7 {

typedef struct _Classer_ctx _Classer_ctx;

class Classer
{
public:
    Classer();
    ~Classer();

    static Long alignStructSize(CListTypeInfo fieldTypes, List<FieldInfo>* fieldInfo);

    ClassHandle define(CString name,CListTypeInfo fieldTypes);
    ObjectHandle create(ClassHandle handle);
    Value getField(ObjectHandle oh, CString fieldName);
    void setField(ObjectHandle oh, CString fieldName, CValue val);
    //WrapValue get(ObjectHandle oh, CTypeInfo type, UInt offset);

private:
    _Classer_ctx* m_impl;
};

}

