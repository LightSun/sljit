#pragma once

#include "h7/h7_ctx.h"
#include <map>

namespace h7 {

enum ParamterDescFlag{
    kPD_FLAG_LS             = 0x0001,   /// local stack. every is sizeof(void*)
    kPD_FLAG_DS             = 0x0002,   /// data-stack. every is sizeof(void*)
    kPD_FLAG_RETURN         = 0x0004,   /// only for function return desc
    kPD_FLAG_IMM            = 0x0008,
    kPD_FLAG_OBJECT_FIELD        = 0x0010,  /// indicate it is runtime come from object. like Person p = ...; p.age=18...
    kPD_FLAG_ARRAY_INDEX_DYNAMIC = 0x0020,  /// indcate the array index is not imm. it comes from runtime.
};

struct ArrayLSIds{
    UInt obj;
    UInt data;
    UInt shape;
    UInt curOffset; //runtime in bytes
    UInt subEleSize;

    ArrayLSIds(CList<UInt> ids){
        this->obj        = ids[0];
        this->data       = ids[1];
        this->shape      = ids[2];
        this->curOffset  = ids[3];
        this->subEleSize = ids[4];
    }
    ArrayLSIds(UInt id0){
        this->obj        = id0;
        this->data       = id0 + 1;
        this->shape      = id0 + 2;
        this->curOffset  = id0 + 3;
        this->subEleSize = id0 + 4;
    }
};

struct ParameterInfo{
    UShort type;        /// base param type
    UShort flags {0};   /// ls or ds
    UInt extIdx {0};    /// field idx from object / LS-index of object.
    //SPtr<UInt> extId2s; /// extra ids for array. like arr[2][3][5]
    Long index;         /// index(DS/LS) of the param.
              /// may function addr for func
              /// may be object addr.
              /// if -1 and is return info, means no need return.
              /// compose index: object-data-addr + ((offset-addr) << 24)
    static ParameterInfo make(UShort type, UShort flags, Long index){
        return {type, flags, 0, /*nullptr,*/ index};
    }
    bool isLS() const{return (flags & kPD_FLAG_LS) != 0;}
    bool isDS() const{return (flags & kPD_FLAG_DS) != 0;}
    bool isReturn()const {return (flags & kPD_FLAG_RETURN) != 0;}
    bool isIMM()const{ return (flags & kPD_FLAG_IMM) != 0;}
    bool isFloatLike() const{ TypeInfo ti(type); return ti.isFloatLikeType();}
    bool is64() const{ TypeInfo ti(type); return ti.is64();}
    bool isSigned()const {TypeInfo ti(type); return ti.isSigned();}
    bool isMinSize()const { return (flags & kPD_FLAG_OBJECT_FIELD) != 0;}
    bool isLessThanInt()const {TypeInfo ti(type); return ti.isLessInt();}
    bool isObjectField()const {return (flags & kPD_FLAG_OBJECT_FIELD) != 0;}
    bool isArrayIndexDynamic()const {return (flags & kPD_FLAG_ARRAY_INDEX_DYNAMIC) != 0;}

    void setComposeIndex(int id1, int id2){ index = id1 | (id2 << 24);}
    void getComposeIndex(int* id1, int* id2){
        *id1 = (index & 0xffffff); *id2 = ((index >> 24) & 0xffffff); }
    UInt getLSObjectIndex()const{return extIdx;}
    UInt getFieldIndex()const{return extIdx;}
    void setExtIndex(int ls_idx){extIdx = ls_idx;}

    ArrayLSIds indexAsArrayIds(){return ArrayLSIds((UInt)index);}
    //UInt getArrayIndex(){
    //    if(!extId2s) return extIdx;
        //arr[2][3][5] -> [1][2]    -> 1*(3*5) + 2 * 5
        //                [1][2][3] -> 1*(3*5) + 2 * 5 + 3
    //}
};
using ParamMap = std::map<int, ParameterInfo>;//k,v = ret+param_index,

//deprecated
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
    LOAD_OBJ,
    STORE_OBJ,
    LOAD_OBJ_F,
    STORE_OBJ_F,
    LOAD_C_STR,
    LOAD_ARR,
    LOAD_ARR_E,
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
    kSENT_FLAG_DEBUG       = 0x0008,
};

struct OpExtraInfo{
    ParamMap funcParams;
    ParameterInfo funcRet;
    String imm;
    //for object: obj, data, offsets, for array: obj, data, shape, element-size
    List<UInt> objIdxes;

    ArrayLSIds asArrayLsIds(){return ArrayLSIds(objIdxes);}
};

struct Operand: public ParameterInfo{
    std::unique_ptr<OpExtraInfo> extra;

    void makeLS(){flags = kPD_FLAG_LS;}
    void makeDS(){flags = kPD_FLAG_DS;}
    void makeExtra(){extra = std::make_unique<OpExtraInfo>();}
    ///imm: only for float-like/int-like
    String getIMM()const{return extra ? extra->imm : "";}
    void makeIMMInt(int val){
        makeIMM(kType_int32, std::to_string(val));
    }
    void makeIMMDouble(double val){
        makeIMM(kType_double, std::to_string(val));
    }
    void makeIMM(int _type, CString val){
        type = _type;
        flags = kPD_FLAG_IMM;
        makeExtra();
        extra->imm = val;
    }
    //-----------------------
    Operand(){}
    Operand(Operand& src){
        operator=(src);
    }
    Operand(const Operand& src){
        operator=(src);
    }
    Operand& operator=(const Operand& src){
        this->type = src.type;
        this->flags = src.flags;
        this->index = src.index;
        if(src.extra){
            extra = std::make_unique<OpExtraInfo>();
            *extra = *src.extra;
        }
        return *this;
    }
    Operand& operator=(Operand& src){
        this->type = src.type;
        this->flags = src.flags;
        this->index = src.index;
        if(src.extra){
            extra = std::make_unique<OpExtraInfo>();
            *extra = *src.extra;
        }
        return *this;
    }
};

struct Sentence{
    OpCode op {NONE};
    int flags {0};
    Operand ip;    ///current var
    Operand left;
    Operand right;
    using Type = std::shared_ptr<Sentence>;

    static Type New(){
        return std::make_shared<Sentence>();
    }
    static std::shared_ptr<Sentence> NewCall(const void* func_ptr){
        auto sent = Sentence::New();
        sent->op = OpCode::CALL;
        sent->setValidFlags(1);
        sent->ip.index = (Long)func_ptr;
        sent->ip.extra = std::make_unique<OpExtraInfo>();
        return sent;
    }
    void addFunctionParameter(const ParameterInfo& pi){
        ip.extra->funcParams[(int)ip.extra->funcParams.size()] = std::move(pi);
    }
    void setFunctionReturn(const ParameterInfo& pi){
        ip.extra->funcRet = pi;
    }
    bool isDebug()const {return hasFlag(kSENT_FLAG_DEBUG);}
    bool hasFlag(int flag)const {return (flags & flag) == flag;}
    void setValidFlagsAll(){
        flags = kSENT_FLAG_VALID_IP | kSENT_FLAG_VALID_LEFT | kSENT_FLAG_VALID_RIGHT;
    }
    bool isFlags2()const{return flags == (kSENT_FLAG_VALID_IP
                                    | kSENT_FLAG_VALID_LEFT);}
    bool isFlags1()const{return flags == kSENT_FLAG_VALID_IP;}
    void setValidFlags(int c){
        switch (c) {
        case 1: flags = kSENT_FLAG_VALID_IP; break;
        case 2: flags = kSENT_FLAG_VALID_IP | kSENT_FLAG_VALID_LEFT; break;
        case 3: flags = kSENT_FLAG_VALID_IP | kSENT_FLAG_VALID_LEFT
                    | kSENT_FLAG_VALID_RIGHT; break;
        }
    }
    //all data from ds
    void makeSimple3DS(int type, CULongArray3 indexArr);
    //1 LS and 2 DS
    void makeSimple1LS2DS(CIntArray3 types, CULongArray3 indexArr);
    //load object
    void makeLoadObjectDS(Long index, CListUInt objIdxes);
    //load object field
    void makeLoadObjectField(CListUInt regs_obj, int type, int lsIdx, int fieldIdx);
    ///store object field:: type: field and LS type. lsIdx: LS-idx. fieldIdx: field idx of object
    void makeStoreObjectField(CListUInt regs_obj, int type, int lsIdx, int fieldIdx);
    //cast type
    void makeTypeCast2LS(int srcType, Long srcIdx, int dstType, Long dstIdx);
    //assign
    void makeAssignByIMM(int dstType, Long dstIdx, int immType,CString imm);
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
    std::shared_ptr<Function> func;

    FuncStatement():Statement(kSTAT_FUNC_INLINE){}
};

}
