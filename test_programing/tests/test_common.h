#pragma once

#include "h7/Instruction.h"
#include "h7/DataStack.h"

namespace h7 {

static void printFloat(float val){
    printf("printFloat >> v = %g\n", val);
}
static void printInt(int val){
    printf("printInt >> val = %d\n", val);
}
static void printChar(char val){
    printf("printChar >> val = %d\n", val);
}

static inline Sentence::Type addStat_printDSInt(int type, int idx){
    auto sent = Sentence::NewCall((void*)printInt);
    sent->addFunctionParameter(ParameterInfo::make(
                                   type, kPD_FLAG_DS, idx));
    return sent;
}
static inline Sentence::Type addStat_printLSInt(int type, int idx){
    auto sent = Sentence::NewCall((void*)printInt);
    sent->addFunctionParameter(ParameterInfo::make(
                type, kPD_FLAG_LS, idx));
    return sent;
}

static inline Sentence::Type addStat_printLSChar(int type, int idx){
    auto sent = Sentence::NewCall((void*)printChar);
    sent->addFunctionParameter(ParameterInfo::make(
                type, kPD_FLAG_LS, idx));
    return sent;
}

typedef void (SLJIT_FUNC *Func_Pt)(void* ptr);;
class SljitFunc{
public:
    void enter(){
        C = sljit_create_compiler(NULL, NULL);

        //as Pt1: has a float . we need a temp float register.
        sljit_emit_enter(C, 0, SLJIT_ARGS1(VOID, P),
                         6, 4,
                         4, 0,
                         1024);
    }
    void exit(){
        sljit_emit_return_void(C);
        code = sljit_generate_code(C);
        codeSize = sljit_get_generated_code_size(C);
        sljit_free_compiler(C);
    }
    void run(){
        void* ptr = ds.getDataPtr();
        auto func = (Func_Pt)code;
        func(ptr);
    }
public:
    DataStack ds{3};
    struct sljit_compiler *C;
    size_t codeSize {0};
    void* code {nullptr};
};


}
