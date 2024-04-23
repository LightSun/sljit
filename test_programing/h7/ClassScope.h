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

    //fns: field names
    ClassInfo* defineClass(CString name,CListTypeInfo fieldTypes, CListString fns,
                           bool ignoreRedefine = false);
    ClassInfo* defineArray(const TypeInfo& info, bool ignoreRedefine = false);
    ///return string data address.
    char* defineConstString(CString str,UInt* idx){
        return pool.defineConstString(str, idx);
    }
    const char* defineString(CString str, uint64* hash){
        return pool.defineString(str, hash);
    }
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

