#include "CodeGen.h"
#include "sljitLir.h"

#include <stdio.h>
#include <stdlib.h>

using namespace h7;

namespace h7 {
struct _CodeGen_ctx{
    sljit_compiler *C;
    std::vector<SPFunc> funcs;
    _CodeGen_ctx(){
        C = sljit_create_compiler(NULL, NULL);
    }
    SPFunc getFunc(){
        return funcs[funcs.size()-1];
    }
    void beginFunc(CString name, ValueType ret, CList<Param> ps){
        SPFunc func = std::make_shared<Func>();
        func->name = name;
        func->ps = ps;
        func->ret = ret;
        funcs.push_back(func);
        //
        int f_c = 0;
        int i_c = 0;
        int p_c = 0;
        for(size_t i = 0 ; i < ps.size() ; ++i){
            switch(ps[i].val){
            case I32:
            case I64:
                {i_c++; }break;
            case F32:
            case F64:
                {f_c++; }break;
            case PTR:
                {p_c++;} break;
            default:
                {}; //empty
            }
        }
        if(i_c > SLJIT_NUMBER_OF_REGISTERS){
            i_c = SLJIT_NUMBER_OF_REGISTERS;
        }
        if(f_c > SLJIT_NUMBER_OF_FLOAT_REGISTERS){
            f_c = SLJIT_NUMBER_OF_FLOAT_REGISTERS;
        }
        sljit_emit_enter(C, 0, SLJIT_ARGS1(VOID, P),
                         1, i_c, 0, f_c,
                         0);
    }
    void pushIns(Inst ins){
        getFunc()->insts.push_back(ins);
    }
    void gen(){

    }
};
}

CodeGen::CodeGen(){
    m_ptr = new _CodeGen_ctx();
}
void CodeGen::beginFunc(CString name, ValueType ret, CList<Param> ps){

}
void CodeGen::endFunc(){

}
