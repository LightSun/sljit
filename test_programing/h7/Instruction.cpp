#include "h7/Instruction.h"
extern "C"{
#include "sljitLir.h"
}

#include "h7/common/common.h"
#include "h7/common/c_common.h"
#include "h7/DataStack.h"
#include "h7/h7_ctx_impl.h"

#include "h7/RegHelper.h"

using namespace h7;

CodeDesc::~CodeDesc(){
    if(code){
        sljit_free_code(code, NULL);
        code = nullptr;
    }
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

UInt Function::getLSOffset(UInt idx){
    return m_localRI->getLSOffset(idx);
}
UInt Function::getDSOffset(UInt idx){
    return m_localRI->getDSOffset(idx);
}
UInt Function::getOffset(UInt idx, bool ls_or_ds){
    return ls_or_ds ? m_localRI->getLSOffset(idx) : m_localRI->getDSOffset(idx);
}
int Function::allocLocal(){
    if(!m_localRI){
        m_localRI = RegisterIndexer::New(localSize);
    }
    return m_localRI->allocLocalIdx();
}
ListUI Function::allocLocalN(int n){
    if(!m_localRI){
        m_localRI = RegisterIndexer::New(localSize);
    }
    ListUI ret;
    for(int i = 0 ; i < n ; ++i){
        ret.push_back(m_localRI->allocLocalIdx(true));
    }
    return ret;
}
int Function::allocLSForArray(){
    //5 count of LS
    int id = m_localRI->allocLocalIdx(true);
    H7_ASSERT(m_localRI->allocLocal());
    H7_ASSERT(m_localRI->allocLocal());
    H7_ASSERT(m_localRI->allocLocal());
    H7_ASSERT(m_localRI->allocLocal());
    return id;
}
int Function::getCurrentLocalIndex(){
    return m_localRI ? m_localRI->getCurrentIdx() : -1;
}

//input: data-arr. all element offset is 8*N. return in S1
String Function::compile(CodeDesc* out){
    struct sljit_compiler *C;
    if(compiler){
        C = (struct sljit_compiler *)compiler;
    }else{
        C = sljit_create_compiler(NULL, NULL);
        sljit_emit_enter(C, 0, SLJIT_ARGS2(VOID, P, P),
                         6, 2, 4, 0, localSize);
    }
    //
    const int size = (int)body.size();
    for(int i = 0; i < size ; ++i){
        auto& st = body[i];
        String msg;
        switch (st->type) {
        case kSTAT_EASY:{
            msg = genEasy(C, st);
        }break;
        case kSTAT_FUNC_INLINE:{
            msg = genInline(C, st);
        }break;

        default:
            gError_throwFmt("statement type(%d) is not support!", st->type);
        }
        if(!msg.empty()){
            return msg;
        }
    }
    if(!compiler){
        sljit_emit_return_void(C);
        out->code = sljit_generate_code(C);
        out->size = sljit_get_generated_code_size(C);
        sljit_free_compiler(C);
    }
    return "";
}

String Function::genInline(void* c,SPStatement st){
    struct sljit_compiler *C = (sljit_compiler*)c;
    FuncStatement* fst = (FuncStatement*)st.get();
    auto func = fst->func.get();
    func->compiler = C;

    //fst->func;
    //raw: all func from DS. -> ls+ds
    //RegStack use parent.
    //ls:
    //ds:
    //sub-func find all operand (from param). change index.
    //sub-func find all operand (from LS),and add offset.

    return "";
}

String Function::genEasy(void* c,SPStatement _st){
    struct sljit_compiler *C = (sljit_compiler*)c;
    EasyStatement* est = (EasyStatement*)_st.get();
    //
    SLJITHelper sh(C, &m_regStack, getRegisterIndexer());
    //st->sent_
    auto& st = est->sent_;
    switch (st->op) {

    case OpCode::ASSIGN:{
        sh.emitAssign(st);
    }break;

    case OpCode::CAST:{
        sh.emitCast(st);
    }break;

    case OpCode::ADD:{
        sh.emitAdd(st);
    }break;

    case OpCode::CALL:{
        sh.emitCall(st);
    }break;

    case OpCode::LOAD_OBJ:{
        sh.emitLoadObject(st);
    }break;

    case OpCode::STORE_OBJ:{
        sh.emitStoreObject(st);
    }break;

    case OpCode::LOAD_OBJ_F:{
        sh.emitLoadField(st);
    }break;

    case OpCode::STORE_OBJ_F:{
        sh.emitStoreField(st);
    }break;

    case OpCode::NEW_ARRAY:{
        //TODO
    }break;

    case OpCode::LOAD_ARR:{
        sh.emitLoadArray(st);
    }break;

    case OpCode::LOAD_ARR_E:{
        sh.emitLoadArrayElement(st);
    }break;

    case OpCode::LOAD_C_STR:{
        sh.emitLoadCStr(st);
    }break;

    default:
        gError_throwFmt("genEasy>> wrong op = %d", st->op);
    }
    m_regStack.reset();
    return "";
}

//TODO to resolve inline functions.(be careful of nested func.)
void Function::updateParamIndex(){
    const int size = (int)body.size();
    for(int i = 0; i < size ; ++i){
        auto& st = body[i];
        //st->u
    }
}
