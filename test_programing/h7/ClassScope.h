#pragma once

#include "h7/h7_ctx.h"
#include "h7/ConstPool.h"

namespace h7 {

typedef struct _ClassScope_ctx _ClassScope_ctx;
struct ClassScope{

private:
    _ClassScope_ctx* m_ctx;
    ClassScope* m_parent {nullptr};
    LinkList<ClassScope*> m_children;

public:
    ConstPool pool;

    ClassScope();
    ~ClassScope();

    static ClassScope* enterGlobal();
    static ClassScope* getGlobal();
    static ClassScope* getCurrent();

    static void enter(ClassScope*);
    static void exit();
    static ClassInfo* newArrayClassInfo(const TypeInfo& info);

    ClassInfo* defineClass(CString name,CListTypeInfo fieldTypes, CListString fns);
    ClassInfo* defineArray(const TypeInfo& info);
    ///return string index.
    UInt defineConstString(CString str){return pool.defineConstString(str);}
    //get class info.
    ClassInfo* getClassInfo(CString clsName);
    //-1 means not found
    int getFieldOffset(CString clsName, CString fn);
    int getFieldOffset(CString clsName, UInt key);
    //null means not found
    FieldInfo* getFieldInfo(CString clsName, CString fn);
    FieldInfo* getFieldInfo(CString clsName, UInt key);

};

}

