#include "Inst_ctx.h"
#include "h7/common/common.h"

using namespace h7;

static inline ParameterInfo* _getParamInfo(ParamMap* pm, int index){
    auto it = pm->find(index);
    return it != pm->end() ? &it->second : nullptr;
}
//return the count of LS changed.
//TODO fix, current is wrong
static inline void _updateIndex(Operand& ip,IFunction* owner){
    auto pinfo = owner->getParamInfo();
    auto pi = _getParamInfo(pinfo, ip.index);
    if(pi != nullptr){
        if(pi->isLS()){
            if(ip.isDS()){
                //src is DS, dst is LS
                ip.index = owner->incNextLSIdx();
                ip.makeLS();
            }else{
                //src is LS, dst is LS
                ip.index = owner->incNextLSIdx();
            }
        }else{
            if(ip.isDS()){
                //src is DS, dst is DS
                ip.index = pi->index;
            }else{
                //src is LS, dst is DS
                H7_ASSERT(false);
            }
        }
    }
}

//all data from ds
void Sentence::makeDSSimple3(int type, CULongArray3 indexArr){
    flags = kSENT_FLAG_VALID_IP | kSENT_FLAG_VALID_LEFT | kSENT_FLAG_VALID_RIGHT;
    ip.makeDS();
    left.makeDS();
    right.makeDS();
    ip.index = indexArr[0];
    left.index = indexArr[1];
    right.index = indexArr[2];
    ip.type = (UShort)type;
    left.type = (UShort)type;
    right.type = (UShort)type;
}

//need alloc LS.
//dsIdxOffset: super function's DS idx count
void Sentence::updateForParamIndex(IFunction* owner){
    if(hasFlag(kSENT_FLAG_VALID_IP)){
        _updateIndex(ip, owner);
    }
    if(hasFlag(kSENT_FLAG_VALID_LEFT)){
        _updateIndex(left, owner);
    }
    if(hasFlag(kSENT_FLAG_VALID_RIGHT)){
        _updateIndex(right, owner);
    }
}
