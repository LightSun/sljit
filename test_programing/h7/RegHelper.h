#pragma once

#include "h7/Inst_ctx.h"
#include "h7/RegisterIndexer.h"
extern "C"{
#include "sljitLir.h"
}

#define REG_NONE (SLJIT_R0 - 1)

namespace h7 {

    //internal enum
    enum{
        kOP_ADD,
        kOP_SUB,
        kOP_MUL,
        kOP_DIV,
        kOP_MOD,
        kOP_LOAD
    };
    class RegStack{
    private:
        enum{
            kLRS_UNKNOWN,
            kLRS_R,
            kLRS_FR,
        };
        int reg {REG_NONE};
        int freg {REG_NONE};
        int lastRegState {kLRS_UNKNOWN};
    public:
        int nextReg(bool _float);
        void backReg();
        void backReg(int c, bool _float);
        void reset(){ reg = REG_NONE; freg = REG_NONE;}
    };

    struct RegDesc{
        bool fs; //float style or not
        int op;
        int r;
        int argType;
        ULong rw;
    };
    class SLJITHelper{
    public:
        SLJITHelper(struct sljit_compiler * c, RegStack* rs,RegisterIndexer* ri):
            C(c), RS(rs), RI(ri){}

        void loadF(int moveType, int dstR, ULong dstw, UInt index, bool ls_or_ds){
            int srcR = ls_or_ds ? SLJIT_MEM1(SLJIT_SP) : SLJIT_MEM1(SLJIT_S0);
            UInt offset = ls_or_ds ? RI->getLSOffset(index) : RI->getDSOffset(index);
            sljit_emit_fop1(C, moveType, dstR, dstw, srcR, offset);
        }
        void load(int moveType, int dstR, ULong dstw, UInt index, bool ls_or_ds){
            int srcR = ls_or_ds ? SLJIT_MEM1(SLJIT_SP) : SLJIT_MEM1(SLJIT_S0);
            UInt offset = ls_or_ds ? RI->getLSOffset(index) : RI->getDSOffset(index);
            sljit_emit_op1(C, moveType, dstR, dstw, srcR, offset);
        }

        void emitAdd(SPSentence s, int targetType);
        void emitCall(SPSentence st);
        void emitAssign(SPSentence st);

    private:
        int emitPrimitive(Operand& src, int targetType);

        RegDesc emitRegDesc(Operand& op, int targetType);
        RegDesc genRetRegDesc(Operand& op, int opBase, int targetType);
        RegDesc genRegDesc(Operand& op);
        //
        void genRegDesc(Operand& op, RegDesc* out);
        ///reg: -1 means gen new reg.
        int loadToReg(Operand& op, int reg);
        void castType(Operand& dst, Operand& src);
        List<RegDesc> genFuncRegDesc(OpExtraInfo* extra);
        RegDesc genRegDesc(ParameterInfo& pi, int baseOp);
        //
        static int getMoveType(int type);
        static int genSLJIT_op(int base, int targetType);
        static int getArgType(int type); /// paramIndex >= 1
        static int getConvType(int srcType, int dstType);

    private:
        struct sljit_compiler *C;
        RegisterIndexer* RI;
        RegStack* RS;
    };
}
