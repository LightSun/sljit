#pragma once

namespace jit {

enum DT{
    kType_S8 = 1,
    kType_U8,
    kType_S16,
    kType_U16,
    kType_S32,
    kType_U32,
    kType_S64,
    kType_U64,
    kType_F32,
    kType_F64,
    kType_P_ARRAY,
    kType_P_MAP,
    kType_P_STRING,
    kType_P_OBJECT,
    kType_P_FUNC,
};

enum DT_VIT{
    kVit_PRIVATE = 1,
    kVit_PROTECTED,
    kVit_PUBLIC,
};

enum DT_FLAG{
    kFLAG_VOLATIVE = 0x0001,
    kFLAG_SORTED = 0x0002,
    kFLAG_REVERSE = 0x0004, //reverse map struct ?, 'kkk...,vvv...' with 'kv,kv,kv...'
};

typedef struct _TypeInfo TypeInfo;
struct _TypeInfo{
    unsigned long long dts;
    union SubInfo{
        TypeInfo* arr;
        TypeInfo** map; //two TypeInfo
        void* obj; //count + (TypeInfo**)
        void* func;//(TypeInfo*) + count + (TypeInfo**) as 'return and params'
    };
    union SubInfo sub;
};

class BaseJitEmiter
{
public:
    BaseJitEmiter();
};

class ClassEmiter{
public:

private:

};

class FuncEmiter{
public:
    FuncEmiter(int pi_c, int pf_c);
private:

};

}

