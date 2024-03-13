#pragma once

#include "h7/h7_ctx.h"
#include "h7/common/common.h"

namespace h7 {

#define REG_NONE 0

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

    CAST,

    RET, BREAK, DEFAULT,
    NEW_LOCAL_PRIMITIVE  // int,long,char ...etc bases.
};

enum SentFlags{
    kSENT_FLAG_VALID_IP    = 0x0001,
    kSENT_FLAG_VALID_LEFT  = 0x0002,
    kSENT_FLAG_VALID_RIGHT = 0x0004,
};

//local_var/func_param
enum{
    kOperand_FLAG_LOCAL      = 0x0001,
    kOperand_FLAG_DATA_STACK = 0x0002,
};

struct Operand{
    UShort flags {0};/// the desc of data
    UShort type;     /// base data-type
    /// when flags has kOperand_FLAG_DATA_STACK, this is index of DataStack
    /// when flags has kOperand_FLAG_LOCAL, this is offset of local-stack.
    /// 0 is return ,1+ is params.
    ULong rw;

    bool isLocal()const{return (flags & kOperand_FLAG_LOCAL) != 0;}
    bool isDataStack()const{return (flags & kOperand_FLAG_DATA_STACK) != 0;}
    void makeLocal(){flags = kOperand_FLAG_LOCAL;}
    void makeDataStack(){flags = kOperand_FLAG_DATA_STACK;}
};

struct Sentence{
    OpCode op;
    int flags {0};
    Operand ip;    ///current var
    Operand left;
    Operand right;

    static std::shared_ptr<Sentence> New(){
        return std::make_shared<Sentence>();
    }
    void makeAllValid(){
        flags = kSENT_FLAG_VALID_IP | kSENT_FLAG_VALID_LEFT | kSENT_FLAG_VALID_RIGHT;
    }
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
enum StatType{
    kSTAT_EASY,
    kSTAT_IF,
    kSTAT_WHILE,
    kSTAT_SWITCH,
    kSTAT_FOR,
};
struct Statement{
    int type;

    Statement(int t):type(t){}
};
struct EasyStatement: public Statement{
    SPSentence sent_;

    EasyStatement():Statement(kSTAT_EASY){}
};

struct IfStatement: public Statement{
    SentBlock if_;
    SentBlock else_;
    List<CaseBlock> elseifs_;

    IfStatement():Statement(kSTAT_IF){}
};

struct WhileStatement: public Statement{
    SentBlock case_;
    SentBlock block_;

    WhileStatement():Statement(kSTAT_WHILE){}
};

struct SwitchStatement: public Statement{
    SPSentence case_;
    List<CaseBlock> blocks_;

    SwitchStatement():Statement(kSTAT_SWITCH){}
};
struct ForStatement: public Statement{
    SentBlock init_;
    SentBlock case_;
    SentBlock end_;
    SentBlock body_;

    ForStatement():Statement(kSTAT_FOR){}
};
using SPStatement = std::shared_ptr<Statement>;

//------------------------------------
class DataStack;
using SPDataStack = std::unique_ptr<DataStack>;

class RegStack{
private:
    int reg {REG_NONE};
    int freg {REG_NONE};
public:
    int nextReg(bool _float);
    void reset();
};
struct RegDesc{
    bool fs; //float style or not
    int op;
    int r;
    ULong rw;
};

struct CodeDesc{
    ULong size;
    void* code {nullptr};
    ~CodeDesc();
    CodeDesc(){}

    static std::unique_ptr<CodeDesc> New(){return std::make_unique<CodeDesc>();}
    /// run the function.
    String run(DataStack* ds);

private:
    __DISABLE_COPY_MOVE(CodeDesc);
};

struct Function{
    int localSize {1024};
    int pCount {0};
    List<SPStatement> body;

public:
    Function(int pCount):pCount(pCount){};
    ///gen function code, if gen failed, return the error msg.
    String compile(CodeDesc* out);

    void addEasyStatment(SPSentence sp){
        auto st = std::make_shared<EasyStatement>();
        st->sent_ = sp;
        body.push_back(st);
    }

private:
    __DISABLE_COPY_MOVE(Function);
    RegStack m_regStack;

private:
    String genEasy(void* compiler,SPStatement st);
    ///return reg id
    int emitPrimitive(void* compiler, Operand& src, int targetType);

    RegDesc emitRegDesc(void *compiler, Operand& op, int targetType);
    RegDesc genRetRegDesc(Operand& op, int opBase, int targetType);
    RegDesc genRetRegDesc(Operand& op);

    void emitAdd(void *compiler, SPSentence st);
};

//func(a, b){c = a+b; return c;}
//c=a+b

}
