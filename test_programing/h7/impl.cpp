#include <stdlib.h>
#include <memory.h>
#include "Classer.h"
#include "h7_alloc.h"
#include "h7_locks.h"
#include "h7_ctx_impl.h"

#include "h7/common/common.h"
#include "h7/utils/h_atomic.h"

using namespace h7;

//in 64. must >= 8
#define _ALIGN_SIZE 8

//-----------------------------

static inline List<int> _new_group(int v){
    return {v};
}
//start: include
//end: exclude
static inline void _add_groups(List<List<int>>& ggs, CListTypeInfo fieldTypes, int start, int end){
    using Group = List<int>;
    int totalSize = 0;
    Group g;
    for(int i = start ; i < end ; ++i){
        auto& ft = fieldTypes[i];
        int tsize = totalSize + ft.virtualSize();
        if(tsize > _ALIGN_SIZE){
            ggs.push_back(g);
            g = Group();
            g.push_back(i);
            totalSize = ft.virtualSize();
        }else if(tsize == _ALIGN_SIZE){
            g.push_back(i);
            ggs.push_back(g);
            g = Group();
            totalSize = 0;
        }
        else{
            g.push_back(i);
            totalSize += ft.virtualSize();
        }
    }
    if(!g.empty()){
        ggs.push_back(g);
    }
}

Long h7::alignStructSize(CListTypeInfo fieldTypes, CListString fns, ClassInfo* out){
    //1, find all 8 size fields.
    //2, mark head and tail if is 8-size or not.
    //3, align non-8-size fields by add.
    using Group = List<int>;
    using GroupS = List<Group>;
    GroupS ggs;
    //
    {
    const int size = fieldTypes.size();
    List<int> aligned_idxes;
    for(int i = 0 ; i < size ; ++i){
        if(fieldTypes[i].isAlignSize(_ALIGN_SIZE)){
            aligned_idxes.push_back(i);
        }
    }
    const int align_size = aligned_idxes.size();
    if(aligned_idxes.empty()){
        int start = 0;
        int end = size;
        _add_groups(ggs, fieldTypes, start, end);
    }else if(aligned_idxes.size() == 1){
        bool head_is_aligned = aligned_idxes[0] == 0;
        bool tail_is_aligned = aligned_idxes[0] == size - 1;
        if(head_is_aligned){
             ggs.push_back(_new_group(0));
             int start = aligned_idxes[0] + 1;
             int end = size;
             _add_groups(ggs, fieldTypes, start, end);
        }else if(tail_is_aligned){
            int start = 0;
            int end = size - 1;
            _add_groups(ggs, fieldTypes, start, end);
            ggs.push_back(_new_group(size - 1));
        }else{
            int start = 0;
            int end = size;
            _add_groups(ggs, fieldTypes, start, end);
        }
    }else{
        // >= 2
        bool head_is_aligned = aligned_idxes[0] == 0;
        bool tail_is_aligned = aligned_idxes[align_size-1] == size - 1;
        int start_idx = 0;
        //check head
        if(head_is_aligned){
            ggs.push_back(_new_group(0));
            int start = aligned_idxes[0] + 1;
            int end = aligned_idxes[1];

            _add_groups(ggs, fieldTypes, start, end);
            start_idx = 1;
        }else{
            int start = 0;
            int end = aligned_idxes[0];
            _add_groups(ggs, fieldTypes, start, end);
        }
        for(int i = start_idx ; i + 1 < align_size ; ++i ){
            ggs.push_back(_new_group(aligned_idxes[i]));
            int start = aligned_idxes[start_idx] + 1;
            int end = aligned_idxes[start_idx + 1];
            _add_groups(ggs, fieldTypes, start, end);
        }
        //check tail
        if(tail_is_aligned){
            ggs.push_back(_new_group(size - 1));
        }else{
            int start_idx = aligned_idxes[align_size-1];
            ggs.push_back(_new_group(start_idx));
            int start = start_idx + 1;
            int end = size;
            _add_groups(ggs, fieldTypes, start, end);
        }
    }
    }
    const int size = ggs.size();
    UInt totalSize = size * _ALIGN_SIZE;
    if(out){
        UInt offset = 0;
        for(int i = 0 ; i < size ; ++i){
            auto& gs = ggs[i];
            int ssize = gs.size();
            offset = i * _ALIGN_SIZE;
            for(int k = 0; k < ssize ; ++k){
                auto& idx = gs[k];
                auto& ft = fieldTypes[idx];
                auto vsize = ft.virtualSize();
                //
                FieldInfo info;
                info.name = fns[idx];
                info.offset = offset;
                info.typeInfo = ft;
                out->putField(info.name, info);
                //(*out->fieldMap)[info.name] = std::move(info);
                //
                offset += vsize;
            }
        }
        out->structSize = totalSize;
    }
    return totalSize;
}
//-------------------------------------------------

void h7::gValue_get(const void* data, UInt type, Value* out){
    switch (type) {
    case kType_bool:{
        out->u8 = (*(UChar*)data) !=0 ? 1 : 0;
    }break;

    case kType_uint8:{
        out->u8 = (*(UChar*)data);
    }break;

    case kType_int8:
    {
            out->u8 = (*(Char*)data);
        }break;
    case kType_int16:
    {
            out->i16 = (*(Short*)data);
        }break;
    case kType_uint16:
    {
            out->u16 = (*(UShort*)data);
        }break;

    case kType_int32:
    {
            out->i32 = (*(Int*)data);
        }break;
    case kType_uint32:
    {
            out->u32 = (*(UInt*)data);
        }break;

    case kType_int64:
    {
            out->i64 = (*(Long*)data);
        }break;
    case kType_uint64:
    {
            out->u64 = (*(ULong*)data);
        }break;

    case kType_float:
    {
            memcpy(&out->f, data, sizeof(Float));
        }break;
    case kType_double:
    {
            memcpy(&out->d, data, sizeof(Double));
        }break;

    default:
        out->ptr = (void*)data;
        //String msg = "gValue_get >> un impl, type = " + std::to_string(type);
        //H7_ASSERT_X(false, msg);
    }
}
void h7::gValue_rawGet(const void* data, UInt type, void* out){

#define _RAW_GET(t) *(t*)out = (*(t*)data); break;
    switch (type) {
    case kType_bool:{
        *(UChar*)out = (*(UChar*)data) !=0 ? 1 : 0;
    }break;

    case kType_uint8: _RAW_GET(UChar);
    case kType_int8: _RAW_GET(Char);

    case kType_int16: _RAW_GET(Short);
    case kType_uint16: _RAW_GET(UShort);

    case kType_int32: _RAW_GET(Int);
    case kType_uint32:_RAW_GET(UInt);

    case kType_int64: _RAW_GET(Long);
    case kType_uint64: _RAW_GET(ULong);

    case kType_float:{
        memcpy(out, data, sizeof (float));
    }break;
    case kType_double:
    {
        memcpy(out, data, sizeof(Double));
    }break;

    default:
        ((void**)out)[0] = (void*)data;
        //String msg = "gValue_get >> un impl, type = " + std::to_string(type);
        //H7_ASSERT_X(false, msg);
    }
#undef _RAW_GET
}
void h7::gValue_rawSet(const void* data, UInt type, void* out){

#define _RAW_SET(t) *(t*)data = (*(t*)out); break;
    switch (type) {
    case kType_bool:{
        *(UChar*)data = (*(UChar*)out) !=0 ? 1 : 0;
    }break;

    case kType_uint8: _RAW_SET(UChar);
    case kType_int8: _RAW_SET(Char);

    case kType_int16: _RAW_SET(Short);
    case kType_uint16: _RAW_SET(UShort);

    case kType_int32: _RAW_SET(Int);
    case kType_uint32:_RAW_SET(UInt);

    case kType_int64: _RAW_SET(Long);
    case kType_uint64: _RAW_SET(ULong);

    case kType_float:{
        memcpy((void*)data, out, sizeof(float));
    }break;
    case kType_double:
    {
        memcpy((void*)data, out, sizeof(Double));
    }break;

    default:
        ((void**)data)[0] = out;
        //String msg = "gValue_get >> un impl, type = " + std::to_string(type);
        //H7_ASSERT_X(false, msg);
    }
#undef _RAW_SET
}

void h7::gValue_set(const void* data, UInt type, Value* v){
    switch (type) {
    case kType_bool:{
        (*(UChar*)data) = v->u8 != 0 ? 1 : 0;
    }break;

    case kType_uint8:{
        (*(UChar*)data) = v->u8;
    }break;

    case kType_int8:
    {
            (*(Char*)data) = v->i8;
        }break;
    case kType_int16:
    {
          (*(Short*)data) = v->i16;
        }break;
    case kType_uint16:
    {
            (*(UShort*)data) = v->u16;
        }break;

    case kType_int32:
    {
            (*(int*)data) = v->i32;
        }break;
    case kType_uint32:
    {
            (*(UInt*)data) = v->u32;
        }break;

    case kType_int64:
    {
            (*(Long*)data) = v->i64;
        }break;
    case kType_uint64:
    {
            (*(ULong*)data) = v->u64;
        }break;

    case kType_float:
    {
            memcpy((void*)data, &v->f, sizeof(Float));
        }break;
    case kType_double:
    {
            memcpy((void*)data, &v->d, sizeof(Double));
        }break;

    default:
        memcpy((void*)data, &v->ptr, sizeof(void*));
        //String msg = "gValue_get >> un impl, type = " + std::to_string(type);
        //H7_ASSERT_X(false, msg);
    }
}

