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
bool Function::allocLocal(){
    if(!m_localRI){
        m_localRI = RegisterIndexer::New(localSize);
    }
    return m_localRI->allocLocal();
}
//input: data-arr. all element offset is 8*N. return in S1
String Function::compile(CodeDesc* out){
    struct sljit_compiler *C;
    if(compiler){
        C = (struct sljit_compiler *)compiler;
    }else{
        C = sljit_create_compiler(NULL, NULL);
        sljit_emit_enter(C, 0, SLJIT_ARGS2(VOID, P, P),
                         4, 2, 4, 0, localSize);
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
    //st->sent_
    auto& st = est->sent_;
    switch (st->op) {
    case OpCode::ADD:{
        emitAdd(C, st);
    }break;

    case OpCode::CALL:{
        emitCall(C, st);
    }break;

    default:
        gError_throwFmt("genEasy>> wrong op = %d", st->op);
    }
    m_regStack.reset();
    return "";
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
    int targetType = TypeInfo::computeAdvanceType(st->left.type, st->right.type);
    //
    H7_ASSERT_X(targetType == (int)t_ret.type, "advance type must = ret type.");
    SLJITHelper sh(C, &m_regStack, getRegisterIndexer());
    sh.emitAdd(st, targetType);
}
void Function::emitCall(void *compiler, SPSentence st){
    struct sljit_compiler *C = (sljit_compiler*)compiler;
    SLJITHelper sh(C, &m_regStack, getRegisterIndexer());
    sh.emitCall(st);
}
void Function::updateParamIndex(){
    const int size = (int)body.size();
    for(int i = 0; i < size ; ++i){
        auto& st = body[i];
        //st->u
    }
}
