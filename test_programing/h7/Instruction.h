#pragma once

#include <map>
#include "h7/Inst_ctx.h"
#include "h7/common/common.h"
#include "h7/RegHelper.h"

namespace h7 {

//------------------------------------
class DataStack;
class RegisterIndexer;
using SPDataStack = std::unique_ptr<DataStack>;
using SPRegisterIndexer = std::unique_ptr<RegisterIndexer>;

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
    using PMap = ParamMap;
    using CParameterInfo = const ParameterInfo&;
public:
    int localSize {1024};
    int pCount {0};
    List<SPStatement> body;
    void* compiler {nullptr};

public:
    Function(int pCount):pCount(pCount){};
    /// gen function code, if gen failed, return the error msg.
    String compile(CodeDesc* out);

    //------------------------

    UInt getLSOffset(UInt idx);
    UInt getDSOffset(UInt idx);
    UInt getOffset(UInt idx, bool ls_or_ds);
    /// int,long,char ...etc bases.
    int allocLocal();
    UIntArray3 allocLocal3();
    /// if not alloc return-1
    int getCurrentLocalIndex();

public:
    void setParameterInfo(int index, CParameterInfo info){
        m_pMap[index] = std::move(info);
    }
    ParameterInfo* getParamterInfo(int index){
        auto it = m_pMap.find(index);
        return it != m_pMap.end() ? &it->second : nullptr;
    }
    void addEasyStatment(SPSentence sp){
        auto st = std::make_shared<EasyStatement>();
        st->sent_ = sp;
        body.push_back(st);
    }
    RegisterIndexer* getRegisterIndexer(){
        return m_localRI.get();
    }

private:
    String genEasy(void* compiler,SPStatement st);
    String genInline(void* compiler,SPStatement st);

//--------------------
    void updateParamIndex();

private:
    __DISABLE_COPY_MOVE(Function);
    RegStack m_regStack;
    SPRegisterIndexer m_localRI;
    PMap m_pMap;
};

//func(a, b){c = a+b; return c;}
//c=a+b

}
