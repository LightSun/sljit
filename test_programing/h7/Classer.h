#pragma once

#include "h7_ctx.h"

namespace h7 {

typedef struct _Classer_ctx _Classer_ctx;

class Classer
{
public:
    Classer();
    ~Classer();

    static Long alignStructSize(CListTypeInfo fieldTypes, CListString fns, ClassInfo* out);

    ClassHandle define(CString name,CListTypeInfo fieldTypes,CListString fns);
    ObjectHandle create(ClassHandle handle);
    Value getField(ObjectHandle oh, CString fieldName);
    void setField(ObjectHandle oh, CString fieldName, CValue val);
    //WrapValue get(ObjectHandle oh, CTypeInfo type, UInt offset);

private:
    Classer& operator=(const Classer&) = delete ;
    Classer& operator=(Classer&) = delete;
    Classer(const Classer&) = delete;
    Classer(Classer&) = delete;

private:
    _Classer_ctx* m_impl;
};

}

