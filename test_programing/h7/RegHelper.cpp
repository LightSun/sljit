#include "RegHelper.h"
#include "h7_ctx_impl.h"
#include "Classer.h"

using namespace h7;

#define LS_R SLJIT_MEM1(SLJIT_SP)
#define DS_R SLJIT_MEM1(SLJIT_S0)
#define LS_OFFSET(i) RI->getLSOffset((UInt)i)
#define DS_OFFSET(i) RI->getDSOffset((UInt)i)

static void printInt(int val){
    printf("SLJITHelper >> printInt: val = %d\n", val);
}

RegDesc SLJITHelper::genRegDesc(Operand& op){
    RegDesc desc;
    genRegDesc(&op, &desc);
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
    //param may be char, short?
    RegDesc rd;
    genRegDesc(&pi, &rd);
    //
    rd.op = genSLJIT_op(baseOp, pi.type);
    rd.argType = getArgType(pi.type);
    return rd;
}
void SLJITHelper::genRegDesc(ParameterInfo* op, RegDesc* out){
    out->fs = op->isFloatLike();
    if(op->isIMM()){
        out->r = SLJIT_IMM;
        out->rw = op->index;
    }else if(op->isLS()){
        out->r = SLJIT_MEM1(SLJIT_SP);
        out->rw = RI->getLSOffset(op->index);
    }else{
        out->r = SLJIT_MEM1(SLJIT_S0);
        out->rw = RI->getDSOffset(op->index);

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
Operand SLJITHelper::castType(int dt, Operand& src){
    if(src.type != dt){
        Operand dstOP;
        dstOP.makeLS();
        dstOP.type = dt;
        dstOP.index = RI->allocLocal();
        castType(dstOP, src);
        return dstOP;
    }
    return src;
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
        int s_r; ULong s_rw;
        int d_r; ULong d_rw;
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
        //printf("convType = %d\n", convType);
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
//-------------------------------------

void SLJITHelper::emitAdd(SPSentence st){
    //dst: ADD_OP and reg.
    if(st->left.type == st->right.type
            && st->right.type == st->ip.type){
        auto op = genSLJIT_op(kOP_ADD, st->ip.type);
        auto ret = genRegDesc(st->ip);
        auto left = genRegDesc(st->left);
        auto right = genRegDesc(st->right);
        if(ret.fs){
            sljit_emit_fop2(C, op, ret.r, ret.rw,
                           left.r, left.rw, right.r, right.rw);
        }else{
            sljit_emit_op2(C, op, ret.r, ret.rw,
                           left.r, left.rw, right.r, right.rw);
        }
        return;
    }
    auto ri_key = RI->save();
    int targetType = TypeInfo::computeAdvanceType(st->left.type, st->right.type);
    List<Operand> opList;
    if(targetType != (int)st->ip.type){
        targetType = TypeInfo::computeAdvanceType(targetType, st->ip.type);
    }
    //ip, left, right
    opList.push_back(castType(targetType, st->ip));
    opList.push_back(castType(targetType, st->left));
    opList.push_back(castType(targetType, st->right));
    auto op = genSLJIT_op(kOP_ADD, targetType);
    //do add
    auto ret = genRegDesc(opList[0]);
    auto left = genRegDesc(opList[1]);
    auto right = genRegDesc(opList[2]);
    if(ret.fs){
        sljit_emit_fop2(C, op, ret.r, ret.rw,
                       left.r, left.rw, right.r, right.rw);
    }else{
        sljit_emit_op2(C, op, ret.r, ret.rw,
                       left.r, left.rw, right.r, right.rw);
    }
    //copy to ret.
    if(targetType != (int)st->ip.type){
        op = genSLJIT_op(kOP_LOAD, targetType);
        if(ret.fs){
            sljit_emit_fop1(C, op, ret.r, ret.rw, SLJIT_FR0, 0);
        }else{
            sljit_emit_op1(C, op, ret.r, ret.rw, SLJIT_R0, 0);
        }
    }
    RI->restore(ri_key);
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
            int reg = RS->nextReg(rd.fs);
            if(rd.fs){
                sljit_emit_fop1(C, rd.op, reg, 0, rd.r, rd.rw);
            }else{
                sljit_emit_op1(C, rd.op, reg, 0, rd.r, rd.rw);
            }
            argFlags |= SLJIT_ARG_VALUE(rd.argType, i);
        }
    }else{
        argRet = SLJIT_ARGS0(VOID);
        for(int i = 0 ; i < (int)list.size() ; ++i){
            auto& rd = list[i];
            int reg = RS->nextReg(rd.fs);
            if(rd.fs){
                sljit_emit_fop1(C, rd.op, reg, 0, rd.r, rd.rw);
            }else{
                sljit_emit_op1(C, rd.op, reg, 0, rd.r, rd.rw);
            }
            argFlags |= SLJIT_ARG_VALUE(rd.argType, i + 1);
        }
    }
    //SLJIT_FAST_CALL?
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
    RS->reset();
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
void SLJITHelper::emitLoadObject(SPSentence st){
    H7_ASSERT_X(st->isFlags1(), "ip must be valid");
    auto& ip = st->ip;
    //all is local index
    H7_ASSERT(ip.extra->objIdxes.size() >= 3);
    auto ls_obj = ip.extra->objIdxes[0];
    auto ls_data = ip.extra->objIdxes[1];
    auto ls_offsets = ip.extra->objIdxes[2];
    //load ptr, offset, real_data addr
    //SP for LS
    if(ip.isLS()){
        sljit_emit_op1(C, SLJIT_MOV, LS_R, RI->getLSOffset(ls_obj),
                       LS_R, RI->getLSOffset(ip.index));
    }else{
        sljit_emit_op1(C, SLJIT_MOV, LS_R, RI->getLSOffset(ls_obj),
                       DS_R, RI->getDSOffset(ip.index));
    }
    //
    int skey = RS->save();
    int reg_tmp = RS->nextReg(false);
    //
    sljit_emit_op1(C, SLJIT_MOV, reg_tmp, 0, LS_R, RI->getLSOffset(ls_obj));
    //load offsets
    sljit_emit_op1(C, SLJIT_MOV, LS_R, RI->getLSOffset(ls_offsets),
                   SLJIT_MEM1(reg_tmp), SLJIT_OFFSETOF(Object, offsets));
    //load real offset
    //sljit_emit_op1(C, SLJIT_MOV_S32, reg_offsets, 0,
    //               SLJIT_MEM1(reg_offsets), left.fieldIdx * sizeof(UInt));
    //load datas
    sljit_emit_op1(C, SLJIT_MOV, LS_R, RI->getLSOffset(ls_data),
                   SLJIT_MEM1(reg_tmp), SLJIT_OFFSETOF(Object, block));
    RS->restore(skey);
}
void SLJITHelper::emitStoreObject(SPSentence st){
    //TODO
    H7_ASSERT_X(false, "latter impl");
}
void SLJITHelper::emitLoadField(SPSentence st){
    //ip: to store on. left is object
    H7_ASSERT_X(st->isFlags2(), "ip and left must be valid");
    H7_ASSERT_X(st->left.isObjectField(), "load field must from Object");
    H7_ASSERT_X(st->left.type == st->ip.type, "emitLoadField >> type must be the same.");
    auto& field_op = st->left;
    int id_data;
    int id_offsets;
    field_op.getComposeIndex(&id_data, &id_offsets);
    auto rd_ip = genRegDesc(st->ip);
    //
    int rs_key = RS->save();
    int reg_data = RS->nextReg(false);
    int reg_offsets = RS->nextReg(false);
    sljit_emit_op1(C, SLJIT_MOV, reg_data, 0, LS_R, RI->getLSOffset(id_data));
    sljit_emit_op1(C, SLJIT_MOV, reg_offsets, 0, LS_R, RI->getLSOffset(id_offsets));
    //---------------------------
    //load offset
    int reg_f = RS->nextReg(false);
    sljit_emit_op1(C, SLJIT_MOV_U32, reg_f, 0,
                       SLJIT_MEM1(reg_offsets), field_op.getFieldIndex() * sizeof(UInt));
    //sljit_emit_op1(C, SLJIT_MOV_U8, SLJIT_R0, 0, SLJIT_MEM2(CELLS, SP), 0);		/* R0 = CELLS[SP] */
    //load real_data. 'data + offset'
    auto op_load = genSLJIT_op(kOP_LOAD, field_op.type);

    // move a 'data+ offset', offset from runtime
    int reg_ip_tmp = RS->nextReg(rd_ip.fs);
    if(rd_ip.fs){
        sljit_emit_fmem(C, op_load | SLJIT_MEM_UNALIGNED, reg_ip_tmp,
                       SLJIT_MEM2(reg_data, reg_f), 0);
        sljit_emit_fop1(C, op_load, rd_ip.r, rd_ip.rw, reg_ip_tmp, 0);
    }else{
        //Shift as size(0 for bytes, 1 for shorts, 2 for 4bytes, 3 for 8bytes)
        sljit_emit_mem(C, op_load | SLJIT_MEM_UNALIGNED, reg_ip_tmp,
                       SLJIT_MEM2(reg_data, reg_f), 0);

        //debug
//        sljit_emit_op1(C, op_load, SLJIT_R0, 0 , reg_ip_tmp, 0);
//        sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(VOID, 32),
//                             SLJIT_IMM, SLJIT_FUNC_ADDR(printInt));

        sljit_emit_op1(C, op_load, rd_ip.r, rd_ip.rw, reg_ip_tmp, 0);
    }
    RS->restore(rs_key);

}
void SLJITHelper::emitStoreField(SPSentence st){
    //ip: object field to store, left is emitter
    H7_ASSERT_X(st->isFlags2(), "ip and left must be valid");
    H7_ASSERT_X(st->ip.isObjectField(), "store field must from Object");
    auto rd_left = genRegDesc(st->left);
    auto& field_op = st->ip;

    int id_data;
    int id_offsets;
    field_op.getComposeIndex(&id_data, &id_offsets);
    //
    int rs_key = RS->save();
    int reg_data = RS->nextReg(false);
    int reg_offsets = RS->nextReg(false);
    sljit_emit_op1(C, SLJIT_MOV, reg_data, 0, LS_R, RI->getLSOffset(id_data));
    sljit_emit_op1(C, SLJIT_MOV, reg_offsets, 0, LS_R, RI->getLSOffset(id_offsets));
    //---------------------------
    //load offset
    int reg_f = RS->nextReg(false);
    sljit_emit_op1(C, SLJIT_MOV_U32, reg_f, 0,
                       SLJIT_MEM1(reg_offsets), field_op.getFieldIndex() * sizeof(UInt));
    //load real_data. 'data + offset'
    auto op_load = genSLJIT_op(kOP_LOAD, field_op.type);

    //how to move a 'data+ offset', offset from runtime
    int reg_ip_tmp = RS->nextReg(rd_left.fs);
    if(rd_left.fs){
        sljit_emit_fop1(C, op_load, reg_ip_tmp, 0, rd_left.r, rd_left.rw);
        sljit_emit_fmem(C, op_load | SLJIT_MEM_STORE | SLJIT_MEM_UNALIGNED, reg_ip_tmp,
                       SLJIT_MEM2(reg_data, reg_f), 0);
    }else{
        //Shift as size(0 for bytes, 1 for shorts, 2 for 4bytes, 3 for 8bytes)
        sljit_emit_op1(C, op_load, reg_ip_tmp, 0, rd_left.r, rd_left.rw);
        sljit_emit_mem(C, op_load | SLJIT_MEM_STORE | SLJIT_MEM_UNALIGNED, reg_ip_tmp,
                       SLJIT_MEM2(reg_data, reg_f), 0);
    }
    RS->restore(rs_key);
}

void SLJITHelper::emitLoadArray(SPSentence st){
    //
    auto& ip = st->ip;
    //all is local index
    H7_ASSERT(ip.extra->objIdxes.size() >= 4);
    auto ls_obj = ip.extra->objIdxes[0];
    auto ls_data = ip.extra->objIdxes[1];
    auto ls_subEleSize = ip.extra->objIdxes[2];
    auto ls_shape = ip.extra->objIdxes[3];
    //load ptr, offset, real_data addr
    //SP for LS
    if(ip.isLS()){
        sljit_emit_op1(C, SLJIT_MOV, LS_R, LS_OFFSET(ls_obj), LS_R, LS_OFFSET(ip.index));
    }else{
        sljit_emit_op1(C, SLJIT_MOV, LS_R, LS_OFFSET(ls_obj), DS_R, DS_OFFSET(ip.index));
    }

    int skey = RS->save();
    int reg_p1 = RS->nextReg(false);
    int reg_p2 = RS->nextReg(false);
    H7_ASSERT(reg_p1 == SLJIT_R0);
    //load ele size. h7::UInt gObject_get_element_size(h7::ObjectPtr ptr, int arrLevel);
    sljit_emit_op1(C, SLJIT_MOV, reg_p1, 0, LS_R, LS_OFFSET(ls_obj));
    sljit_emit_op1(C, SLJIT_MOV, reg_p2, 0, SLJIT_IMM, -1);
    //
    //load datas
    sljit_emit_op1(C, SLJIT_MOV, LS_R, LS_OFFSET(ls_data),
                   SLJIT_MEM1(reg_p1), SLJIT_OFFSETOF(Object, block));
    //load shape
    sljit_emit_op1(C, SLJIT_MOV, LS_R, LS_OFFSET(ls_shape),
                   SLJIT_MEM1(reg_p1), SLJIT_OFFSETOF(Object, offsets));
    //load eleSize
    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS2(W, P, 32),
                     SLJIT_IMM, SLJIT_FUNC_ADDR(gObject_get_element_size));
    sljit_emit_op1(C, SLJIT_MOV, LS_R, LS_OFFSET(ls_eleSize), SLJIT_R0, 0);

    RS->restore(skey);
}

void SLJITHelper::emitLoadArrayElement(SPSentence st){
    //ip is the array element to store, left is parent array.
    H7_ASSERT_X(st->isFlags2(), "ip and left must be valid");
    auto rd_ip = genRegDesc(st->ip);
    //
    auto& ele_op = st->ip;
    auto& arr_op = st->left;
    int id_data;
    int id_childEleSize;
    arr_op.getComposeIndex(&id_data, &id_childEleSize);
    //ext index as field index.

    int rs_key = RS->save();
    // load data and eleSize
    int reg_data = RS->nextReg(false);
    sljit_emit_op1(C, SLJIT_MOV, reg_data, 0, LS_R, LS_OFFSET(id_data));
    //---------------------------
    //compute real-offset
    int reg_f = RS->nextReg(false);
    if(ele_op.isArrayIndexDynamic()){
        sljit_emit_op2(C, SLJIT_MUL, reg_f, 0, LS_R, LS_OFFSET(id_childEleSize),
                           LS_R, LS_OFFSET(arr_op.getFieldIndex()));
    }else{
        sljit_emit_op2(C, SLJIT_MUL, reg_f, 0, LS_R, LS_OFFSET(id_childEleSize),
                           SLJIT_IMM, arr_op.getFieldIndex());
    }
    //load real_data. 'data + offset'
    auto op_load = genSLJIT_op(kOP_LOAD, arr_op.type);
    // move a 'data+ offset', offset from runtime
    int reg_ip_tmp = RS->nextReg(rd_ip.fs);
    if(rd_ip.fs){
        sljit_emit_fmem(C, op_load | SLJIT_MEM_UNALIGNED, reg_ip_tmp,
                       SLJIT_MEM2(reg_data, reg_f), 0);
        sljit_emit_fop1(C, op_load, rd_ip.r, rd_ip.rw, reg_ip_tmp, 0);
    }else{
        //Shift as size(0 for bytes, 1 for shorts, 2 for 4bytes, 3 for 8bytes)
        sljit_emit_mem(C, op_load | SLJIT_MEM_UNALIGNED, reg_ip_tmp,
                       SLJIT_MEM2(reg_data, reg_f), 0);
        sljit_emit_op1(C, op_load, rd_ip.r, rd_ip.rw, reg_ip_tmp, 0);
    }
    RS->restore(rs_key);
}

void SLJITHelper::emitLoadCStr(SPSentence st){
    //TODO
    H7_ASSERT_X(st->isFlags1(), "ip must be valid");
    auto& ip = st->ip;
}
