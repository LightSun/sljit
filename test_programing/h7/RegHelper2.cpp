#include "RegHelper.h"

using namespace h7;

namespace h7 {
struct RegStackState{
    int reg;
    int freg;
    int lastRegState;
};
enum{
    kLRS_UNKNOWN,
    kLRS_R,
    kLRS_FR,
};
struct _RegStack_ctx{
    std::map<int,RegStackState> stateMap;

    int reg {REG_NONE};
    int freg {REG_NONE};
    int lastRegState {kLRS_UNKNOWN};

    int save(){
        RegStackState s;
        s.reg = reg;
        s.freg = freg;
        s.lastRegState = lastRegState;
        int c = (int)stateMap.size();
        stateMap[c] = std::move(s);
        return c;
    }
    void restore(int key){
        auto it = stateMap.find(key);
        if(it != stateMap.end()){
            reg = it->second.reg;
            freg = it->second.freg;
            lastRegState = it->second.lastRegState;
            stateMap.erase(it);
        }
    }
    int nextReg(bool _float){
        if(_float){
            if(freg == SLJIT_NUMBER_OF_FLOAT_REGISTERS - 1){
                H7_ASSERT_X(false, "wrong reg state");
            }
            freg ++;
            lastRegState = kLRS_FR;
            return freg;
        }else{
            if(reg == SLJIT_NUMBER_OF_REGISTERS - 1){
                H7_ASSERT_X(false, "wrong reg state");
            }
            reg ++;
            lastRegState = kLRS_R;
            return reg;
        }
    }
    void backReg(){
        switch (lastRegState) {
        case kLRS_R: {reg -- ; lastRegState = kLRS_UNKNOWN; }break;
        case kLRS_FR: {freg -- ; lastRegState = kLRS_UNKNOWN; }break;
        case kLRS_UNKNOWN:
        default:
            H7_ASSERT_X(false, "RegStack >> unknown 'lastRegState'");
        }
    }
    void backReg(int c, bool _float){
        if(_float){
            H7_ASSERT(freg >= c);
            freg -= c;
            lastRegState = kLRS_UNKNOWN;
        }else{
            H7_ASSERT(reg >= c);
            reg -= c;
            lastRegState = kLRS_UNKNOWN;
        }
    }
     void reset(){
         reg = REG_NONE;
         freg = REG_NONE;
         lastRegState = kLRS_UNKNOWN;
     }
};
}

RegStack::RegStack(){
    m_ptr = new _RegStack_ctx();
}
RegStack::~RegStack(){
    if(m_ptr){
        delete m_ptr;
        m_ptr = nullptr;
    }
}
int RegStack::save(){
    return m_ptr->save();
}
void RegStack::restore(int key){
    m_ptr->restore(key);
}
int RegStack::nextReg(bool _float){
    return m_ptr->nextReg(_float);
}
void RegStack::backReg(){
    m_ptr->backReg();
}
void RegStack::backReg(int c, bool _float){
    m_ptr->backReg(c, _float);
}
void RegStack::reset(){
    m_ptr->reset();
}

int SLJITHelper::getArgType(int type){
    TypeInfo ti(type);
    H7_ASSERT(ti.virtualSize() >= (int)sizeof(int));
    int actType;
    switch (type) {
    case kType_double: actType = SLJIT_ARG_TYPE_F64; break;
    case kType_float: actType = SLJIT_ARG_TYPE_F32; break;
    case kType_int32: actType = SLJIT_ARG_TYPE_32; break;
    case kType_uint32: actType = SLJIT_ARG_TYPE_32; break;
    case kType_int64: actType = SLJIT_ARG_TYPE_W; break;
    case kType_uint64: actType = SLJIT_ARG_TYPE_W; break;
    default: actType = SLJIT_ARG_TYPE_P; break;
    }
    //return SLJIT_ARG_VALUE(actType, paramIndex);
    return actType;
}
int SLJITHelper::getConvType(int srcType, int dstType){
    TypeInfo ti_src(srcType);
    TypeInfo ti_dst(dstType);
    if(ti_dst.isFloatLikeType()){
        if(ti_dst.is64()){
            switch (srcType) {
            case kType_int32:
                return SLJIT_CONV_F64_FROM_S32;
            case kType_uint32:
                return SLJIT_CONV_F64_FROM_U32;
            case kType_int64:
                return SLJIT_CONV_F64_FROM_SW;
            case kType_float:
                return SLJIT_CONV_F64_FROM_F32;
            case kType_double:
                return SLJIT_MOV_F64;
            case kType_uint64:
            default:
                return SLJIT_CONV_F64_FROM_UW;
            }
        }else{
            switch (srcType) {
            case kType_int32:
                return SLJIT_CONV_F32_FROM_S32;
            case kType_uint32:
                return SLJIT_CONV_F32_FROM_U32;
            case kType_int64:
                return SLJIT_CONV_F32_FROM_SW;
            case kType_float:
                return SLJIT_MOV_F32;
            case kType_double:
                return SLJIT_CONV_F32_FROM_F64;
            case kType_uint64:
            default:
                return SLJIT_CONV_F32_FROM_UW;
            }
        }
    }else{
        //printf("srcType, dstType = %d, %d\n", srcType, dstType);
        switch (dstType) {
        case kType_int32:{
            //dst-> int32
            switch (srcType) {
            case kType_float:
                return SLJIT_CONV_S32_FROM_F32;
            case kType_double:
                return SLJIT_CONV_S32_FROM_F64;

            case kType_int8:
                return SLJIT_MOV_S8;
            case kType_uint8:
                return SLJIT_MOV_U8;
            case kType_int16:
                return SLJIT_MOV_S16;
            case kType_uint16:
                return SLJIT_MOV_U16;

            case kType_int32:
                return SLJIT_MOV_S32;
            case kType_uint32:
                return SLJIT_MOV_U32;
            case kType_int64:
            case kType_uint64:
            default:
                return SLJIT_MOV;
            }
        }break;

        case kType_uint32:{
            //dst-> uint32
            switch (srcType) {
            case kType_float:
                return SLJIT_CONV_SW_FROM_F32;
            case kType_double:
                return SLJIT_CONV_SW_FROM_F64;

            case kType_int8:
                return SLJIT_MOV_S8;
            case kType_uint8:
                return SLJIT_MOV_U8;
            case kType_int16:
                return SLJIT_MOV_S16;
            case kType_uint16:
                return SLJIT_MOV_U16;

            case kType_int32:
                return SLJIT_MOV_U32;
            case kType_uint32:
                return SLJIT_MOV_U32;
            case kType_int64:
            case kType_uint64:
            default:
                return SLJIT_MOV;
            }
        }break;

        case kType_int64:{
            //dst-> int64
            switch (srcType) {
            case kType_float:
                return SLJIT_CONV_SW_FROM_F32;
            case kType_double:
                return SLJIT_CONV_SW_FROM_F64;

            case kType_int8:
                return SLJIT_MOV_S8;
            case kType_uint8:
                return SLJIT_MOV_U8;
            case kType_int16:
                return SLJIT_MOV_S16;
            case kType_uint16:
                return SLJIT_MOV_U16;

            case kType_int32:
                return SLJIT_MOV_U32;
            case kType_uint32:
                return SLJIT_MOV_U32;
            case kType_int64:
            case kType_uint64:
            default:
                return SLJIT_MOV;
            }
        }break;

        default:
        case kType_uint64:{
            //dst-> uint64
            switch (srcType) {
            case kType_float:
                return SLJIT_CONV_SW_FROM_F32;
            case kType_double:
                return SLJIT_CONV_SW_FROM_F64;

            case kType_int8:
                return SLJIT_MOV_S8;
            case kType_uint8:
                return SLJIT_MOV_U8;
            case kType_int16:
                return SLJIT_MOV_S16;
            case kType_uint16:
                return SLJIT_MOV_U16;

            case kType_int32:
                return SLJIT_MOV_U32;
            case kType_uint32:
                return SLJIT_MOV_U32;
            case kType_int64:
            case kType_uint64:
            default:
                return SLJIT_MOV;
            }
        }break;
        }
    }
    char buf[128];
    snprintf(buf, 128, "src_t, dst_t = (%d, %d)", srcType, dstType);
    auto str = String(buf);
    H7_ASSERT_X(false, str);
    return SLJIT_MOV;
}

//--------------------------------------
int SLJITHelper::getMoveType(int type){
    int moveType;
    switch (type) {
    case kType_int8:
        { moveType = SLJIT_MOV_S8; }break;
    case kType_int16:
        { moveType = SLJIT_MOV_S16; }break;
    case kType_int32:
        { moveType = SLJIT_MOV_S32; }break;
    case kType_int64:
        { moveType = SLJIT_MOV; }break;
    case kType_float:
        { moveType = SLJIT_MOV_F32; }break;
    case kType_double:
        { moveType = SLJIT_MOV_F64; }break;

    case kType_bool:
        { moveType = SLJIT_MOV_S8; }break;
    case kType_uint8:
        { moveType = SLJIT_MOV_U8; }break;
    case kType_uint16:
        { moveType = SLJIT_MOV_U16; }break;
    case kType_uint32:
        { moveType = SLJIT_MOV_U32; }break;
    case kType_uint64:{
        { moveType = SLJIT_MOV; }break;
    }break;
    default:
        { moveType = SLJIT_MOV_P; }break;
    }
    return moveType;
}
int SLJITHelper::genSLJIT_op(int base, int targetType){
    TypeInfo ti(targetType);
    if(ti.isFloatLikeType()){
        if(ti.virtualSize() > (int)sizeof(Float)){
            switch (base) {
            case kOP_ADD:{return SLJIT_ADD_F64;}
            case kOP_SUB:return SLJIT_SUB_F64;
            case kOP_MUL:return SLJIT_MUL_F64;
            case kOP_DIV:return SLJIT_DIV_F64;

            case kOP_MOD:{
                H7_ASSERT_X(false, "mod op for float/double is not support.");
            }break;

            case kOP_LOAD:{
                return SLJIT_MOV_F64;
            }break;

            default:
                H7_ASSERT(false);
            }
        }else{
            switch (base) {
            case kOP_ADD:return SLJIT_ADD_F32;
            case kOP_SUB:return SLJIT_SUB_F32;
            case kOP_MUL:return SLJIT_MUL_F32;
            case kOP_DIV:return SLJIT_DIV_F32;

            case kOP_MOD:{
                H7_ASSERT_X(false, "mod op for float/double is not support.");
            }break;

            case kOP_LOAD:{
                return SLJIT_MOV_F32;
            }break;

            default:
                H7_ASSERT(false);
            }
        }
    }else{
        if(ti.virtualSize() > (int)sizeof(Int)){
            switch (base) {
            case kOP_ADD: return SLJIT_ADD;
            case kOP_SUB: return SLJIT_SUB;
            case kOP_MUL: return SLJIT_MUL;
            case kOP_DIV: return ti.isSigned() ? SLJIT_DIV_SW : SLJIT_DIV_UW; //no remainder.
            case kOP_MOD: return ti.isSigned() ? SLJIT_DIVMOD_SW : SLJIT_DIVMOD_UW;
            case kOP_LOAD:{
                return SLJIT_MOV;
            }break;

            default:
                H7_ASSERT(false);
            }
        }else{
            switch (base) {
            case kOP_ADD: return SLJIT_ADD32;
            case kOP_SUB: return SLJIT_SUB32;
            case kOP_MUL: return SLJIT_MUL32;
            case kOP_DIV: return ti.isSigned() ? SLJIT_DIV_S32 : SLJIT_DIV_U32; //no remainder.
            case kOP_MOD: return ti.isSigned() ? SLJIT_DIVMOD_S32 : SLJIT_DIVMOD_U32;
            case kOP_LOAD:{
                if(ti.virtualSize() == (int)sizeof(Int)){
                    return ti.isSigned() ? SLJIT_MOV_S32 : SLJIT_MOV_U32;
                }else if(ti.virtualSize() == (int)sizeof(Short)){
                    return ti.isSigned() ? SLJIT_MOV_S16 : SLJIT_MOV_U16;
                }else if(ti.virtualSize() == (int)sizeof(Char)){
                    return ti.isSigned() ? SLJIT_MOV_S8 : SLJIT_MOV_U8;
                }
            }break;

            default:
                H7_ASSERT(false);
            }
        }
    }
    //can't reach here
    return 0;
}

