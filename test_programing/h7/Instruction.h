#pragma once

#include "h7_ctx.h"

namespace h7 {
//using ShareFunc = std::shared_ptr<Func>;

enum OpCode{
    NEW,
    ASSIGN, //=
    CALL,
    LOAD,
    STORE,
    //++, --
    INC, DEC,
    //
    ADD, SUB, MUL, DIV, MOD,
    // & | ~ !
    AND, OR, XOR, NOT,
    //signed/unsigned shift left/right
    SHL, SHR, ASHL, ASHR,
    RET, BREAK, DEFAULT
};

enum SentFlags{
    kSENT_FLAG_VALID_IP    = 0x0001,
    kSENT_FLAG_VALID_LEFT  = 0x0002,
    kSENT_FLAG_VALID_RIGHT = 0x0004,
};

struct Operand{
    int type;
    int r;
    Long rw;
};

struct Sentence{
    OpCode op;
    int flags {0};
    Operand ip;
    Operand left;
    Operand right;
};
using SPSentence = std::shared_ptr<Sentence>;

struct SentBlock{
    List<SPSentence> sents_;
};

struct CaseBlock{
    SentBlock case_;
    SentBlock block_;
};
//---------------------
struct Statement{

};
struct EasyStatement: public Statement{
    SPSentence sent_;
};

struct IfStatement: public Statement{
    SentBlock if_;
    SentBlock else_;
    List<CaseBlock> elseifs_;
};

struct WhileStatement: public Statement{
    SentBlock case_;
    SentBlock block_;
};

struct SwitchStatement: public Statement{
    SPSentence case_;
    List<CaseBlock> blocks_;
};
struct ForStatement: public Statement{
    SentBlock init_;
    SentBlock case_;
    SentBlock end_;
    SentBlock body_;
};
using SPStatement = std::shared_ptr<Statement>;

//------------------------------------
struct FunctionParameter{
    String name;
    String type;
};
typedef struct FunctionContext FunctionContext;
struct Function{
    List<FunctionParameter> params;
    FunctionContext* runCtx;
    List<SPStatement> body;
};

//func(a, b){c = a+b; return c;}
//c=a+b

}
