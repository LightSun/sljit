#include "RegHelper.h"

using namespace h7;

int SLJITHelper::emitPrimitive(Operand& src, int targetType){
//    if(src.isDataStack()){
//        H7_ASSERT_X((int)src.index < pCount, "index param failed.");
//    }
    //
    int moveType = getMoveType(src.type);
    if(moveType == SLJIT_MOV_F32){
        TypeInfo ti(targetType);
        H7_ASSERT(ti.isFloatLikeType());
        int reg = RS->nextReg(true);
        loadF(moveType, reg, 0, src.index, src.isLS());

        if(targetType == kType_double){
            sljit_emit_fop1(C, SLJIT_CONV_F64_FROM_F32, reg, 0, reg, 0);
        }
        return reg;
    }else if(moveType == SLJIT_MOV_F64){
        H7_ASSERT(targetType == kType_double);
        int reg = RS->nextReg(true);
        loadF(moveType, reg, 0, src.index, src.isLS());
        return reg;
    }
    else{
        int reg = RS->nextReg(false);
        load(moveType, reg, 0, src.index, src.isLS());
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
    desc.op = genSLJIT_op(opBase, targetType);
    return desc;
}
RegDesc SLJITHelper::genRegDesc(Operand& op){
    RegDesc desc;
    genRegDesc(op, &desc);
    return desc;
}
List<RegDesc> SLJITHelper::genFuncRegDesc(OpExtraInfo* extra){
    List<RegDesc> ret;
    if(extra){
        //now just care out-func. latter care-about inner.
        //return
        if(extra->funcRet.isReturn()){
            ret.push_back(genRegDesc(extra->funcRet, kOP_LOAD));
        }
        //params
        auto& fpi = extra->funcParams;
        auto it = fpi.begin();
        for(; it != fpi.end() ; ++it){
            ret.push_back(genRegDesc(it->second, kOP_LOAD));
        }
    }else{
        H7_ASSERT(false);
    }
    return ret;
}
RegDesc SLJITHelper::genRegDesc(ParameterInfo& pi, int baseOp){
    RegDesc rd;
    rd.fs = pi.isFloatLike();
    if(pi.isIMM()){
        H7_ASSERT_X(!pi.isReturn(), "genRegDesc >> return-type can't be IMM.");
        rd.r = SLJIT_IMM;
        rd.rw = pi.index; //as value
    }else if(pi.isLS()){
        rd.r = SLJIT_MEM1(SLJIT_SP);
        rd.rw = RI->getLSOffset(pi.index);
    }else{
        rd.r = SLJIT_MEM1(SLJIT_S0);
        rd.rw = RI->getDSOffset(pi.index);
    }
    rd.op = genSLJIT_op(baseOp, pi.type);
    rd.argType = getArgType(pi.type);
    return rd;
}
void SLJITHelper::genRegDesc(Operand& op, RegDesc* out){
    out->fs = op.isFloatLike();
    if(op.isIMM()){
        out->r = SLJIT_IMM;
        out->rw = op.index;
    }else if(op.isLS()){
        out->r = SLJIT_MEM1(SLJIT_SP);
        out->rw = RI->getLSOffset(op.index);
    }else{
        out->r = SLJIT_MEM1(SLJIT_S0);
        out->rw = RI->getDSOffset(op.index);
    }
}
int SLJITHelper::loadToReg(Operand& op, int reg){
    auto rd = genRegDesc(op);
    //if reg not assigned. gen new.
    if(reg < 0){
        reg = RS->nextReg(op.isFloatLike());
    }
    if(op.isIMM()){
        auto imm = op.getIMM();
        if(op.isFloatLike()){
            if(op.is64()){
                sljit_emit_fset64(C, reg, std::stod(imm));
            }else{
                sljit_emit_fset32(C, reg, std::stof(imm));
            }
        }else{
            auto lp = genSLJIT_op(kOP_LOAD, op.type);
            if(op.type == kType_uint64){
                sljit_emit_op1(C, lp, reg, 0, SLJIT_IMM, std::stoull(imm));
            }else{
                sljit_emit_op1(C, lp, reg, 0, SLJIT_IMM, std::stoll(imm));
            }
        }
    }else{
        auto lp = genSLJIT_op(kOP_LOAD, op.type);
        if(op.isFloatLike()){
            sljit_emit_fop1(C, lp, reg, 0, rd.r, rd.rw);
        }else{
            sljit_emit_op1(C, lp, reg, 0, rd.r, rd.rw);
        }
    }
    return reg;
}
void SLJITHelper::castType(Operand& dst, Operand& src){
    //src can be imm.
    //but dst not.
    H7_ASSERT(!dst.isIMM());
    auto rd_dst = genRegDesc(dst);
    if(src.isIMM()){
        if(dst.isFloatLike()){
            if(dst.is64()){
                sljit_emit_fop1(C, SLJIT_MOV_F32, rd_dst.r, rd_dst.rw,
                                SLJIT_IMM, std::stof(src.getIMM()));
            }else{
                sljit_emit_fop1(C, SLJIT_MOV_F64, rd_dst.r, rd_dst.rw,
                                SLJIT_IMM, std::stod(src.getIMM()));
            }
        }else{
            if(dst.type == kType_uint64){
                sljit_emit_op1(C, SLJIT_MOV, rd_dst.r, rd_dst.rw,
                               SLJIT_IMM, std::stoull(src.getIMM()));
            }else{
                sljit_emit_op1(C, SLJIT_MOV, rd_dst.r, rd_dst.rw,
                               SLJIT_IMM, std::stoll(src.getIMM()));
            }
        }
    }else{
        auto key_save = RS->save();
        auto rd_src = genRegDesc(src);
        bool srcIsLessInt = src.isMinSize() && src.isLessThanInt();
        bool dstIsLessInt = dst.isMinSize() && dst.isLessThanInt();
        int s_r, s_rw;
        int d_r, d_rw;
        int src_tt, dst_tt;
        if(srcIsLessInt){
            s_r = loadToReg(src, -1);
            s_rw = 0;
            src_tt = kType_int32;
        }else{
            s_r = rd_src.r;
            s_rw = rd_src.rw;
            src_tt = src.type;
        }
        if(dstIsLessInt){
            d_r = RS->nextReg(src.isFloatLike());
            d_rw = 0;
            dst_tt = kType_int32;
        }else{
            d_r = rd_dst.r;
            d_rw = rd_dst.rw;
            dst_tt = dst.type;
        }
        int convType = getConvType(src_tt, dst_tt);
        if(dst.isFloatLike() || src.isFloatLike()){
            sljit_emit_fop1(C, convType, d_r, d_rw, s_r, s_rw);
        }else{
            sljit_emit_op1(C, convType, d_r, d_rw, s_r, s_rw);
        }
        if(dstIsLessInt){
            //copy data to dst.
            auto loadT = genSLJIT_op(kOP_LOAD, dst.type);
            if(dst.isFloatLike()){
                sljit_emit_fop1(C, loadT, rd_dst.r, rd_dst.rw, d_r, d_rw);
            }else{
                sljit_emit_op1(C, loadT, rd_dst.r, rd_dst.rw, d_r, d_rw);
            }
        }
        RS->restore(key_save);
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
void SLJITHelper::emitCall(SPSentence st){
    //add(a,b,c) -> [ret,a,b,c]
    //H7_ASSERT(RI->allocLocal());
    //ip is ret, left is func.
    //1, put param to reg
    RS->reset();
    bool hasRet = st->ip.extra->funcRet.isReturn();
    auto list = genFuncRegDesc(st->ip.extra.get());
    int argFlags = 0;
    int argRet;
    if(hasRet){
        argRet = SLJIT_ARG_RETURN(list[0].argType);
        for(int i = 1 ; i < (int)list.size() ; ++i){
            auto& rd = list[i];
            int reg;
            if(rd.fs){
                reg = RS->nextReg(true);
                sljit_emit_fop1(C, rd.op, reg, 0, rd.r, rd.rw);
            }else{
                reg = RS->nextReg(false);
                sljit_emit_op1(C, rd.op, reg, 0, rd.r, rd.rw);
            }
            argFlags |= SLJIT_ARG_VALUE(rd.argType, i);
        }
    }else{
        argRet = SLJIT_ARGS0(VOID);
        for(int i = 0 ; i < (int)list.size() ; ++i){
            auto& rd = list[i];
            int reg;
            if(rd.fs){
                reg = RS->nextReg(true);
                sljit_emit_fop1(C, rd.op, reg, 0, rd.r, rd.rw);
            }else{
                reg = RS->nextReg(false);
                sljit_emit_op1(C, rd.op, reg, 0, rd.r, rd.rw);
            }
            argFlags |= SLJIT_ARG_VALUE(rd.argType, i + 1);
        }
    }
    //SLJIT_FAST_CALL
    sljit_emit_icall(C, SLJIT_CALL, argRet | argFlags,
                     SLJIT_IMM, SLJIT_FUNC_ADDR(st->ip.index));
    //copy result to ret-reg
    if(hasRet){
        auto& rd = list[0];
        if(rd.fs){
            sljit_emit_fop1(C, rd.op, rd.r, rd.rw, SLJIT_FR0, 0);
        }else{
            //be careful. remainder in R1
            sljit_emit_op1(C, rd.op, rd.r, rd.rw, SLJIT_R0, 0);
        }
    }
}
void SLJITHelper::emitAssign(SPSentence st){
    //ip and left.
    //int a = b , int a = 1.
    H7_ASSERT_X(st->isFlags2(), "for assign: must have ip and left.");
    castType(st->ip, st->left);
}
void SLJITHelper::emitCast(SPSentence st){
    //ip and left.
    //int a = (int)b + c , int a = (int)1.2 + c.
    H7_ASSERT_X(st->isFlags2(), "for assign: must have ip and left.");
    castType(st->ip, st->left);
}

//---------------------------------------------
