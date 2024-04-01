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
    typedef struct _RegStack_ctx _RegStack_ctx;
    class RegStack{
    private:
        _RegStack_ctx* m_ptr;
    public:
        RegStack();
        ~RegStack();

        int save();
        void restore(int);
        int nextReg(bool _float);
        void backReg();
        void backReg(int c, bool _float);
        void reset();
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

        void emitAdd(SPSentence s);
        void emitCall(SPSentence st);
        void emitAssign(SPSentence st);
        void emitCast(SPSentence st);
        void emitLoadObject(SPSentence st);
        void emitStoreObject(SPSentence st);
        void emitLoadField(SPSentence st);
        void emitStoreField(SPSentence st);
        void emitLoadArray(SPSentence st);

        void emitLoadCStr(SPSentence st);

    private:
        RegDesc genRegDesc(Operand& op);
        //
        void genRegDesc(ParameterInfo* op, RegDesc* out);
        ///reg: -1 means gen new reg.
        int loadToReg(Operand& op, int reg);
        void castType(Operand& dst, Operand& src);
        Operand castType(int dstType, Operand& src);
        List<RegDesc> genFuncRegDesc(OpExtraInfo* extra);
        RegDesc genRegDesc(ParameterInfo& pi, int baseOp);
        //
        static int getMoveType(int type);
        static int genSLJIT_op(int base, int targetType);
        static int getArgType(int type); /// paramIndex >= 1
        static int getConvType(int srcType, int dstType);

    private:
        struct sljit_compiler *C;
        RegStack* RS;
        RegisterIndexer* RI;
    };
}
