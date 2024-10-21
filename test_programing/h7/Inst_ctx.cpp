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
    auto pi = _getParamInfo(pinfo, (int)ip.index);
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
void Sentence::makeSimple3DS(int type, CULongArray3 indexArr){
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
void Sentence::makeSimple1LS2DS(CIntArray3 types, CULongArray3 indexArr){
    flags = kSENT_FLAG_VALID_IP | kSENT_FLAG_VALID_LEFT | kSENT_FLAG_VALID_RIGHT;
    ip.makeLS();
    left.makeDS();
    right.makeDS();
    ip.index = indexArr[0];
    left.index = indexArr[1];
    right.index = indexArr[2];
    ip.type = (UShort)types[0];
    left.type = (UShort)types[1];
    right.type = (UShort)types[2];
}

void Sentence::makeLoadObjectDS(Long index, CListUInt objIdxes){
    flags = kSENT_FLAG_VALID_IP;
    ip.makeDS();
    ip.index = index;
    ip.type = kType_object;
    ip.makeExtra();
    ip.extra->objIdxes = objIdxes;
    op = OpCode::LOAD_OBJ;
}

void Sentence::makeLoadObjectField(CListUInt regs_obj, int type, int lsIdx, int fieldIdx){
    flags = kSENT_FLAG_VALID_IP | kSENT_FLAG_VALID_LEFT;
    ip.makeLS();
    ip.type = (u16)type;
    ip.index = lsIdx;
    left.setComposeIndex(regs_obj[1], regs_obj[2]);
    left.flags = kPD_FLAG_LS | kPD_FLAG_OBJECT_FIELD;
    left.type = (u16)type;
    left.setExtIndex(fieldIdx);
    op = OpCode::LOAD_OBJ_F;
}

void Sentence::makeStoreObjectField(CListUInt regs_obj, int type, int lsIdx, int fieldIdx){
    //ip is field.
    flags = kSENT_FLAG_VALID_IP | kSENT_FLAG_VALID_LEFT;
    left.makeLS();
    left.type = (u16)type;
    left.index = lsIdx;
    ip.setComposeIndex(regs_obj[1], regs_obj[2]);
    ip.flags = kPD_FLAG_LS | kPD_FLAG_OBJECT_FIELD;
    ip.type = (u16)type;
    ip.setExtIndex(fieldIdx);
    op = OpCode::STORE_OBJ_F;
}

void Sentence::makeTypeCast2LS(int srcType, Long srcIdx, int dstType, Long dstIdx){
    flags = kSENT_FLAG_VALID_IP | kSENT_FLAG_VALID_LEFT;
    ip.makeLS();
    ip.type = (u16)dstType;
    ip.index = dstIdx;
    left.makeLS();
    left.type = (u16)srcType;
    left.index = srcIdx;
    op = OpCode::CAST;
}

void Sentence::makeAssignByIMM(int dstType, Long dstIdx, int immType, CString imm){
    flags = kSENT_FLAG_VALID_IP | kSENT_FLAG_VALID_LEFT;
    ip.makeLS();
    ip.type = (u16)dstType;
    ip.index = dstIdx;
    left.makeIMM(immType, imm);
    op = OpCode::ASSIGN;
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
