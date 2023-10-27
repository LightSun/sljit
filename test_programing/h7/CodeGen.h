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

};

struct Func{
    CString name;
    ValueType ret;
    List<Param> ps;
    List<Instruction> insts;
};

class CodeGen
{
public:
    void beginFunc(CString name, ValueType ret, CList<Param> ps);
    void endFunc();
    void pushIns();

private:

};

}

