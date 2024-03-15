#pragma once

#include "h7/h7_ctx.h"
#include <map>

namespace h7 {

struct ParamterInfo{
    bool ls; /// ls or ds
    int superParamIdx; /// index(DS/LS) of the param.
                  /// raw idx from super func. if need.

    static ParamterInfo make(int idx, bool ls){
        ParamterInfo info;
        info.ls = ls;
        info.superParamIdx = idx;
        return info;
    }
    bool isLocal() const{return ls;}
};
using ParamMap = std::map<int, ParamterInfo>;//k,v = ret+param_index,

class IFunction{
public:
    virtual ParamMap* getParamInfo() = 0;
    /// return next ls idx. then auto add.
    UInt incNextLSIdx(){
        while (m_parent != nullptr) {
            m_parent = m_parent->getParent();
        }
        return m_parent->incNextLSIdx0();
    }
    void setParent(IFunction* p){m_parent = p;}
    IFunction* getParent(){return m_parent;}
protected:
    virtual UInt incNextLSIdx0() = 0;

private:
    IFunction* m_parent {nullptr};
};

enum OpCode{
    NONE,
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
    /// when flags has kOperand_FLAG_LOCAL, this is index of local-stack.
    /// 0 is return ,1+ is params.
    ULong index;

    bool isLocal()const{return (flags & kOperand_FLAG_LOCAL) != 0;}
    bool isDataStack()const{return (flags & kOperand_FLAG_DATA_STACK) != 0;}
    void makeLocal(){flags = kOperand_FLAG_LOCAL;}
    void makeDataStack(){flags = kOperand_FLAG_DATA_STACK;}
};

struct Sentence{
    OpCode op {NONE};
    int flags {0};
    Operand ip;    ///current var
    Operand left;
    Operand right;

    static std::shared_ptr<Sentence> New(){
        return std::make_shared<Sentence>();
    }
    bool hasFlag(int flag){
        return (flags & flag) == flag;
    }
    void setValidFlagsAll(){
        flags = kSENT_FLAG_VALID_IP | kSENT_FLAG_VALID_LEFT | kSENT_FLAG_VALID_RIGHT;
    }
    //all data from ds
    void makeDSSimple3(int type, CULongArray3 indexArr);
    /// return LS change count
    void updateForParamIndex(IFunction* owner);
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
    kSTAT_FUNC_INLINE,
};
struct Statement{
    int type;

    Statement(int t):type(t){}
};
using SPStatement = std::shared_ptr<Statement>;

struct EasyStatement: public Statement{
    SPSentence sent_;

    EasyStatement():Statement(kSTAT_EASY){}

    void updateParamIndex();
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

struct Function;

struct FuncStatement: public Statement{
    std::unique_ptr<Function> func;

    FuncStatement():Statement(kSTAT_FUNC_INLINE){}
};

}
