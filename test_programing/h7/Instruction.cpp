#include "h7/Instruction.h"
extern "C"{
#include "sljitLir.h"
}

#include "h7/common/common.h"
#include "h7/common/c_common.h"
#include "h7/DataStack.h"
#include "h7/h7_ctx_impl.h"

using namespace h7;

enum{
    kOP_ADD,
    kOP_SUB,
    kOP_MUL,
    kOP_DIV,
    kOP_MOD,
};
static inline int _typeToSLJIT_MoveType(int type);
static inline int _genSLJIT_op(int base, int targetType);

CodeDesc::~CodeDesc(){
    if(code){
        sljit_free_code(code, NULL);
        code = nullptr;
    }
}
int RegStack::nextReg(bool _float){
    if(_float){
        if(reg == REG_NONE){
            reg = SLJIT_FR0;
            return reg;
        }
        if(reg == SLJIT_NUMBER_OF_FLOAT_REGISTERS - 1){
            H7_ASSERT_X(false, "wrong reg state");
        }
        reg ++;
        return reg;
    }else{
        if(reg == REG_NONE){
            reg = SLJIT_R0;
            return reg;
        }
        if(reg == SLJIT_NUMBER_OF_REGISTERS - 1){
            H7_ASSERT_X(false, "wrong reg state");
        }
        reg ++;
        return reg;
    }
}
void RegStack::reset(){
    reg = REG_NONE;
    freg = REG_NONE;
}
//
typedef void (SLJIT_FUNC *Func_Pt)(void* ptr);;
String CodeDesc::run(DataStack* ds){
    H7_ASSERT(code);
    void* ptr = ds->getDataPtr();
    auto func = (Func_Pt)code;
    func(ptr);
    return "";
}
//input: data-arr. all element offset is 8*N. return in S1
String Function::compile(CodeDesc* out){

    struct sljit_compiler *C = sljit_create_compiler(NULL, NULL);
    sljit_emit_enter(C, 0, SLJIT_ARGS2(VOID, P, P),
                     4, 2, 4, 0, localSize);
    //
    const int size = body.size();
    for(int i = 0; i < size ; ++i){
        auto& st = body[i];
        String msg;
        switch (st->type) {
        case kSTAT_EASY:{
            msg = genEasy(C, st);
        }break;

        default:
            gError_throwFmt("statement type(%d) is not support!", st->type);
        }
        if(!msg.empty()){
            return msg;
        }
    }
    sljit_emit_return_void(C);
    out->code = sljit_generate_code(C);
    out->size = sljit_get_generated_code_size(C);
    sljit_free_compiler(C);
    return "";
}

String Function::genEasy(void* c,SPStatement _st){
    struct sljit_compiler *C = (sljit_compiler*)c;
    EasyStatement* est = (EasyStatement*)_st.get();
    //st->sent_
    auto& st = est->sent_;
    switch (st->op) {
    case OpCode::ADD:{
        emitAdd(C, st);
    }break;

    default:
        gError_throwFmt("genEasy>> wrong op = %d", st->op);
    }
    m_regStack.reset();
    return "";
}
int Function::emitPrimitive(void* compiler, Operand& src, int targetType){
    struct sljit_compiler *C = (sljit_compiler*)compiler;
    if(src.isDataStack()){
        H7_ASSERT_X((int)src.rw < pCount, "index param failed.");
    }
    //
    int moveType = _typeToSLJIT_MoveType(src.type);
    if(moveType == SLJIT_MOV_F32){
        TypeInfo ti(targetType);
        H7_ASSERT(ti.isFloatLikeType());
        int reg = m_regStack.nextReg(true);
        if(src.isLocal()){
            //local stack
            sljit_emit_fop1(C, moveType, reg, 0,
                           SLJIT_MEM1(SLJIT_SP), src.rw);
        }else{
            sljit_emit_fop1(C, moveType, reg, 0,
                           SLJIT_MEM1(SLJIT_S0), src.rw * sizeof(void*));
        }
        if(targetType == kType_double){
            sljit_emit_fop1(C, SLJIT_CONV_F64_FROM_F32, reg, 0, reg, 0);
        }
        return reg;
    }else if(moveType == SLJIT_MOV_F64){
        H7_ASSERT(targetType == kType_double);
        int reg = m_regStack.nextReg(true);
        if(src.isLocal()){
            sljit_emit_fop1(C, moveType, reg, 0,
                           SLJIT_MEM1(SLJIT_SP), src.rw);
        }else{
            sljit_emit_fop1(C, moveType, reg, 0,
                           SLJIT_MEM1(SLJIT_S0), src.rw * sizeof(void*));
        }
        return reg;
    }
    else{
        int reg = m_regStack.nextReg(false);
        if(src.isLocal()){
            sljit_emit_op1(C, moveType, reg, 0,
                           SLJIT_MEM1(SLJIT_SP), src.rw);
        }else{
            sljit_emit_op1(C, moveType, reg, 0,
                           SLJIT_MEM1(SLJIT_S0), src.rw * sizeof(void*));
        }
        //convert if need.
        if(targetType == kType_float){
            TypeInfo ti(src.type);
            int op = ti.isSigned() ? SLJIT_CONV_F32_FROM_S32 : SLJIT_CONV_F32_FROM_U32;
            int reg2 = m_regStack.nextReg(true);
            sljit_emit_fop1(C, op, reg2, 0, reg, 0);
            reg = reg2;
        }else if(targetType == kType_double){
            TypeInfo ti(src.type);
            int op = ti.isSigned() ? SLJIT_CONV_F64_FROM_SW : SLJIT_CONV_F64_FROM_UW;
            int reg2 = m_regStack.nextReg(true);
            sljit_emit_fop1(C, op, reg2, 0, reg, 0);
            reg = reg2;
        }
        return reg;
    }
}
RegDesc Function::emitRegDesc(void *compiler, Operand& op, int targetType){
    struct sljit_compiler *C = (sljit_compiler*)compiler;

    RegDesc desc;
    if(op.type != targetType){
        desc.r = emitPrimitive(C, op, targetType);
        desc.rw = 0;
    }else if(op.isLocal()){
        desc.r = SLJIT_MEM1(SLJIT_SP);
        desc.rw = op.rw;
    }else{
        desc.r = SLJIT_MEM1(SLJIT_S0);
        desc.rw = op.rw * sizeof(void*);
    }
    return desc;
}
RegDesc Function::genRetRegDesc(Operand& op){
    RegDesc desc;
    if(op.isLocal()){
        desc.r = SLJIT_MEM1(SLJIT_SP);
        desc.rw = op.rw;
    }else{
        desc.r = SLJIT_MEM1(SLJIT_S0);
        desc.rw = op.rw * sizeof(void*);
    }
    return desc;
}
RegDesc Function::genRetRegDesc(Operand& op, int opBase, int targetType){
    RegDesc desc;
    desc.fs = false;
    if(op.type != targetType){
        TypeInfo ti(targetType);
        desc.fs = ti.isFloatLikeType();
        desc.r = m_regStack.nextReg(desc.fs);
        desc.rw = 0;
    }else if(op.isLocal()){
        desc.r = SLJIT_MEM1(SLJIT_SP);
        desc.rw = op.rw;
    }else{
        desc.r = SLJIT_MEM1(SLJIT_S0);
        desc.rw = op.rw * sizeof(void*);
    }
    desc.op = _genSLJIT_op(opBase, targetType);
    return desc;
}
void Function::emitAdd(void *compiler, SPSentence st){
    struct sljit_compiler *C = (sljit_compiler*)compiler;
    //dst: ADD_OP and reg.
    TypeInfo t_left(st->left.type);
    TypeInfo t_right(st->right.type);
    TypeInfo t_ret(st->ip.type);
    H7_ASSERT_X(t_left.isPrimitiveType(), "left must be primitive.");
    H7_ASSERT_X(t_right.isPrimitiveType(), "right must be primitive.");
    H7_ASSERT_X(t_ret.isPrimitiveType(), "ip must be primitive.");
    int leftSize = t_left.virtualSize();
    int rightSize = t_right.virtualSize();
    int maxSize = HMAX(leftSize, rightSize);
    bool hasSigned = t_left.isSigned() || t_right.isSigned();
    bool hasFT = t_left.isFloatLikeType() || t_right.isFloatLikeType();
    int targetType = TypeInfo::computePrimitiveType(hasFT, hasSigned, maxSize);
    //
    H7_ASSERT_X(targetType == (int)t_ret.type, "advance type must = ret type.");
    //
    auto ret = genRetRegDesc(st->ip, kOP_ADD, targetType);
    auto left = emitRegDesc(C, st->left, targetType);
    auto right = emitRegDesc(C, st->right, targetType);
    if(ret.fs){
        sljit_emit_fop2(C, ret.op, ret.r, ret.rw,
                       left.r, left.rw, right.r, right.rw);
    }else{
        sljit_emit_op2(C, ret.op, ret.r, ret.rw,
                       left.r, left.rw, right.r, right.rw);
    }
}

//---------------------------------------------
int _typeToSLJIT_MoveType(int type){
    int moveType;
    switch (type) {
    case kType_int8:
        { moveType = SLJIT_MOV_S8; }break;
    case kType_int16:
        { moveType = SLJIT_MOV_S16; }break;
    case kType_int32:
        { moveType = SLJIT_MOV_S32; }break;
    case kType_int64:
        { moveType = SLJIT_MOV; }break;
    case kType_float:
        { moveType = SLJIT_MOV_F32; }break;
    case kType_double:
        { moveType = SLJIT_MOV_F64; }break;

    case kType_bool:
        { moveType = SLJIT_MOV_S8; }break;
    case kType_uint8:
        { moveType = SLJIT_MOV_U8; }break;
    case kType_uint16:
        { moveType = SLJIT_MOV_U16; }break;
    case kType_uint32:
        { moveType = SLJIT_MOV_U32; }break;
    case kType_uint64:{
        { moveType = SLJIT_MOV; }break;
    }break;
    default:
        { moveType = SLJIT_MOV_P; }break;
    }
    return moveType;
}
int _genSLJIT_op(int base, int targetType){
    TypeInfo ti(targetType);
    if(ti.isFloatLikeType()){
        if(ti.virtualSize() > (int)sizeof(Float)){
            switch (base) {
            case kOP_ADD:{return SLJIT_ADD_F64;}
            case kOP_SUB:return SLJIT_SUB_F64;
            case kOP_MUL:return SLJIT_MUL_F64;
            case kOP_DIV:return SLJIT_DIV_F64;

            case kOP_MOD:{
                H7_ASSERT_X(false, "mod op for float/double is not support.");
            }break;

            default:
                H7_ASSERT(false);
            }
        }else{
            switch (base) {
            case kOP_ADD:return SLJIT_ADD_F32;
            case kOP_SUB:return SLJIT_SUB_F32;
            case kOP_MUL:return SLJIT_MUL_F32;
            case kOP_DIV:return SLJIT_DIV_F32;

            case kOP_MOD:{
                H7_ASSERT_X(false, "mod op for float/double is not support.");
            }break;

            default:
                H7_ASSERT(false);
            }
        }
    }else{
        if(ti.virtualSize() > (int)sizeof(Int)){
            switch (base) {
            case kOP_ADD: return SLJIT_ADD;
            case kOP_SUB: return SLJIT_SUB;
            case kOP_MUL: return SLJIT_MUL;
            case kOP_DIV: return ti.isSigned() ? SLJIT_DIV_SW : SLJIT_DIV_UW; //no remainder.
            case kOP_MOD: return ti.isSigned() ? SLJIT_DIVMOD_SW : SLJIT_DIVMOD_UW;
            default:
                H7_ASSERT(false);
            }
        }else{
            switch (base) {
            case kOP_ADD: return SLJIT_ADD32;
            case kOP_SUB: return SLJIT_SUB32;
            case kOP_MUL: return SLJIT_MUL32;
            case kOP_DIV: return ti.isSigned() ? SLJIT_DIV_S32 : SLJIT_DIV_U32; //no remainder.
            case kOP_MOD: return ti.isSigned() ? SLJIT_DIVMOD_S32 : SLJIT_DIVMOD_U32;
            default:
                H7_ASSERT(false);
            }
        }
    }
    //can't reach here
    return 0;
}
