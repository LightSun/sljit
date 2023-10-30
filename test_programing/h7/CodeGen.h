#pragma once
#include "h7_ctx.h"

namespace h7 {

enum ValueType{
    VOID, //can't be param
    I32,
    I64,
    PTR,
    F32,
    F64,
};
typedef struct class_s              class_t;
typedef struct class_s              object_t;
typedef struct {
    class_t         *isa;          // EVERY object must have an ISA pointer (8 bytes on a 64bit system)
    union {                        // union takes 8 bytes on a 64bit system
        long long  n;              // integer slot
        double     f;              // float/double slot
        object_t    *p;            // ptr to object slot
    };
} value_t;

enum OpCode{
    MOV_REG_REG,     // reg to reg
    MOV_SREG_REG,
    MOV_REG_SREG,
    MOV_CONSTI32_REG,  // const to reg
    MOV_CONSTI64_REG,  //
    MOV_CONSTP_REG,    // ptr
    MOV_CONSTF32_REG,  // float
    MOV_CONSTF64_REG,  // double
    CALL,
    LOADI32,
    LOADI64,
    LOADP,
    LOADF32,
    LOADF64,
    STOREI32,
    STOREI64,
    STOREP,
    STOREF32,
    STOREF64,
};

struct Param{
    String name;
    ValueType val;
};

struct Instruction{
    OpCode opc;
    String left;
    String right;
    String dst;
};

struct Func{
    String name;
    ValueType ret;
    List<Param> ps;
    List<Instruction> insts;
};
using Inst = const Instruction&;
using SPFunc = std::shared_ptr<Func>;

typedef struct _CodeGen_ctx _CodeGen_ctx;
class CodeGen
{
public:
    CodeGen();
    ~CodeGen();
    void beginFunc(CString name, ValueType ret, CList<Param> ps);
    void endFunc();
    void pushIns(Inst ins);

private:
    _CodeGen_ctx* m_ptr;
};

}

