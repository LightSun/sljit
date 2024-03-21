#pragma once

#include "h7/h7_ctx.h"
#include <map>

namespace h7 {

enum ParamterDescFlag{
    kPD_FLAG_LS         = 0x0001,   /// local stack. every is sizeof(void*)
    kPD_FLAG_DS         = 0x0002,   /// data-stack. every is sizeof(void*)
    kPD_FLAG_RETURN     = 0x0004,   /// only for function return desc
    kPD_FLAG_IMM        = 0x0008,
    kPD_FLAG_MIN_SIZE   = 0x0010,  /// means. char.size = sizeof(char)...etc
};

struct ParameterInfo{
    UShort type;      ///base param type
    UShort flags {0}; /// ls or ds
    Long index; /// index(DS/LS) of the param.
              /// may be from super func. if need.
              /// if -1 and is return info, means no need return.

    bool isLS() const{return (flags & kPD_FLAG_LS) != 0;}
    bool isDS() const{return (flags & kPD_FLAG_DS) != 0;}
    bool isReturn()const {return (flags & kPD_FLAG_RETURN) != 0;}
    bool isIMM()const{ return (flags & kPD_FLAG_IMM) != 0;}
    bool isFloatLike() const{ TypeInfo ti(type); return ti.isFloatLikeType();}
    bool is64() const{ TypeInfo ti(type); return ti.is64();}
    bool isMinSize()const{(flags & kPD_FLAG_MIN_SIZE) != 0;}
    bool isLessThanInt()const {TypeInfo ti(type); return ti.isLessInt();}
};
using ParamMap = std::map<int, ParameterInfo>;//k,v = ret+param_index,

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
    CALL,   // add(a,b,c)
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

struct OpExtraInfo{
    ParamMap funcParams;
    ParameterInfo funcRet;
    String imm;
};

struct Operand: public ParameterInfo{
    std::unique_ptr<OpExtraInfo> extra;

    void makeLS(){flags = kPD_FLAG_LS;}
    void makeDS(){flags = kPD_FLAG_DS;}
    void makeExtra(){extra = std::make_unique<OpExtraInfo>();}
    ///imm: only for float-like/int-like
    String getIMM()const{return extra ? extra->imm : "";}
    void makeIMMInt(int val){
        type = kType_int32;
        flags = kPD_FLAG_IMM;
        makeExtra();
        extra->imm = std::to_string(val);
    }
    void makeIMMDouble(double val){
        type = kType_double;
        flags = kPD_FLAG_IMM;
        makeExtra();
        extra->imm = std::to_string(val);
    }
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
    void setValidFlags(int c){
        switch (c) {
        case 1: flags = kSENT_FLAG_VALID_IP; break;
        case 2: flags = kSENT_FLAG_VALID_IP | kSENT_FLAG_VALID_LEFT; break;
        case 3: flags = kSENT_FLAG_VALID_IP | kSENT_FLAG_VALID_LEFT
                    | kSENT_FLAG_VALID_RIGHT; break;
        }
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
