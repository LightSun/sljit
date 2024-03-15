#include "RegHelper.h"

using namespace h7;

static inline int _typeToSLJIT_MoveType(int type);
static inline int _genSLJIT_op(int base, int targetType);

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
int SLJITHelper::emitPrimitive(Operand& src, int targetType){
//    if(src.isDataStack()){
//        H7_ASSERT_X((int)src.index < pCount, "index param failed.");
//    }
    //
    int moveType = _typeToSLJIT_MoveType(src.type);
    if(moveType == SLJIT_MOV_F32){
        TypeInfo ti(targetType);
        H7_ASSERT(ti.isFloatLikeType());
        int reg = RS->nextReg(true);
        loadF(moveType, reg, 0, src.index, src.isLocal());

        if(targetType == kType_double){
            sljit_emit_fop1(C, SLJIT_CONV_F64_FROM_F32, reg, 0, reg, 0);
        }
        return reg;
    }else if(moveType == SLJIT_MOV_F64){
        H7_ASSERT(targetType == kType_double);
        int reg = RS->nextReg(true);
        loadF(moveType, reg, 0, src.index, src.isLocal());
        return reg;
    }
    else{
        int reg = RS->nextReg(false);
        load(moveType, reg, 0, src.index, src.isLocal());
        //convert if need.
        if(targetType == kType_float){
            TypeInfo ti(src.type);
            int op = ti.isSigned() ? SLJIT_CONV_F32_FROM_S32 : SLJIT_CONV_F32_FROM_U32;
            int reg2 = RS->nextReg(true);
            sljit_emit_fop1(C, op, reg2, 0, reg, 0);
            reg = reg2;
        }else if(targetType == kType_double){
            TypeInfo ti(src.type);
            int op = ti.isSigned() ? SLJIT_CONV_F64_FROM_SW : SLJIT_CONV_F64_FROM_UW;
            int reg2 = RS->nextReg(true);
            sljit_emit_fop1(C, op, reg2, 0, reg, 0);
            reg = reg2;
        }
        return reg;
    }
}
RegDesc SLJITHelper::emitRegDesc(Operand& op, int targetType){
    RegDesc desc;
    if(op.type != targetType){
        desc.r = emitPrimitive(op, targetType);
        desc.rw = 0;
    }else{
        genRegDesc(op, &desc);
    }
    return desc;
}
RegDesc SLJITHelper::genRetRegDesc(Operand& op, int opBase, int targetType){
    RegDesc desc;
    desc.fs = false;
    if(op.type != targetType){
        TypeInfo ti(targetType);
        desc.fs = ti.isFloatLikeType();
        desc.r = RS->nextReg(desc.fs);
        desc.rw = 0;
    }else{
        genRegDesc(op, &desc);
    }
    desc.op = _genSLJIT_op(opBase, targetType);
    return desc;
}
RegDesc SLJITHelper::genRetRegDesc(Operand& op){
    RegDesc desc;
    genRegDesc(op, &desc);
    return desc;
}
void SLJITHelper::genRegDesc(Operand& op, RegDesc* out){
    if(op.isLocal()){
        out->r = SLJIT_MEM1(SLJIT_SP);
        out->rw = RI->getLSOffset(op.index);
    }else{
        out->r = SLJIT_MEM1(SLJIT_S0);
        out->rw = RI->getDSOffset(op.index);
    }
}
void SLJITHelper::emitAdd(SPSentence st, int targetType){
    auto ret = genRetRegDesc(st->ip, kOP_ADD, targetType);
    auto left = emitRegDesc(st->left, targetType);
    auto right = emitRegDesc(st->right, targetType);
    if(ret.fs){
        sljit_emit_fop2(C, ret.op, ret.r, ret.rw,
                       left.r, left.rw, right.r, right.rw);
    }else{
        sljit_emit_op2(C, ret.op, ret.r, ret.rw,
                       left.r, left.rw, right.r, right.rw);
    }
}

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

