#pragma once

#include "h7_ctx.h"

namespace h7 {
class Func;
class VM;
using ShareFunc = std::shared_ptr<Func>;

VM* vm_new();

enum OpCode{
    MOV_REG_REG,    //reg to reg
    MOV_CONSTI_REG,  // const to reg
    MOV_CONSTS_REG,  // string
    MOV_CONSTP_REG,  // ptr
    MOV_CONSTF_REG,  // float
    MOV_CONSTD_REG,  // double
    NEW,
    CALL,
    LOADI,
    LOADS,
    LOADP,
    LOADF,
    LOADD,
    STOREI,
    STORES,
    STOREP,
    STOREF,
    STORED,
};

enum ObjectType{
    INT,
    FLOAT,
    DOUBLE,
    FUNC,
    PTR
};

struct ManagedObject{
    union{
        long long l;
        float f;
        double d;
        void* ptr;
    } val;
    ObjectType type;
};

struct ObjectPool{
    void put(CString identifier, const ManagedObject& obj);

    Func* owner;
    HashMap<String, ManagedObject> objs;
};

struct OP{
    struct PName{
        String oname; //object name
        String mname; //member name
    };
    PName src;
    PName dst;
};

class Instruction{
public:
    int opCode;
    String n1;
    String n2;
    String nret;
private:
    Func* func;
};

class Func{
public:
    Func(Func* parent);

    void execute();
    void pushParams(CString name, const ManagedObject& obj);
    void pushInstruction(const Instruction& ins);
    void gen();
private:
    String name;
    Func* parent;
    List<Instruction> ins;
    ObjectPool func_params;
    ObjectPool locals;
};

class VM{
public:
    ~VM();

    void beginFunc(Func* func);
    void end();
private:
    Func* global;
    Func* current;
};


}
