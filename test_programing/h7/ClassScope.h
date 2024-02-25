#pragma once

#include "h7/h7_ctx.h"

namespace h7 {

typedef  struct _ClassScope_ctx _ClassScope_ctx;
struct ClassScope{

    ClassScope();
    ~ClassScope();

    static ClassScope* createGlobal();
    static ClassScope* getGlobal();
    static ClassScope* getCurrent();

    static void enter(ClassScope*);
    static void exit();
    static ClassInfo* newArrayClassInfo(const TypeInfo& info);

    ClassInfo* defineClass(CString name,CListTypeInfo fieldTypes, CListString fns);
    ClassInfo* defineArray(const TypeInfo& info);
    //-1 means not found
    int getFieldOffset(CString clsName, CString fn);
    //null means not found
    FieldInfo* getFieldInfo(CString clsName, CString fn);

private:
    _ClassScope_ctx* m_ctx;
    ClassScope* m_parent {nullptr};
    LinkList<ClassScope*> m_children;
};

}

