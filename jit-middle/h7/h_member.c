#include "h_member.h"
#include "h7/common/halloc.h"
#include "h7/h_atomic.h"
#include "h7/hash.h"
#include "h7/h_string.h"

#define __EQ_I(hffi_t, t)\
case hffi_t:{\
    if(src->value._##t == dst->value._##t){\
      return kState_OK; }\
}break;

#define __HASH_I(hffi_t, t)\
case hffi_t:{\
    return fasthash32(&src->value._##t, sizeof(t), seed); \
}break;

#define __DUMP_I(hffi_t, t, fmt)\
case hffi_t:{\
    hstring_appendf(hs, fmt, src->value._##t); \
}break;

static inline void __hmember_init(hmember* arr);

static IObjPtr (Func_copy0)(IObjPtr src1, IObjPtr dst1){
     hmember* src = (hmember*)src1;
     hmember* dst = (hmember*)dst1;
     if(dst == NULL){
         dst = (hmember_p)ALLOC(sizeof(struct hmember));
         __hmember_init(dst);
     }else{
        if(dt_is_pointer(dst->dt)){
            dtype_obj_delete(&dst->dt, dst->value._extra);
        }
     }
     dst->onlyType = src->onlyType;
     dst->dt = src->dt;
     dst->value = src->value;
     return dst;
}

static int (Func_equals0)(IObjPtr src1, IObjPtr dst1){
    int ret = IObject_eqauls_base(src1, dst1);
    if(ret != kState_NEXT){
        return ret;
    }
    hmember* src = (hmember*)src1;
    hmember* dst = (hmember*)dst1;
    if(src->dt != dst->dt){
        return kState_FAILED;
    }
    if(src->onlyType != dst->onlyType){
        return kState_FAILED;
    }
    if(src->onlyType){
        return kState_OK;
    }
    //cmp value
    if(dt_is_pointer(src->dt)){
        return dtype_obj_equals(&src->dt, src->value._extra,
                         dst->value._extra);
    }else{
       DEF_DT_BASE_SWITCH(__EQ_I, src->dt);
    }
    return kState_FAILED;
}

static uint32 (Func_hash0)(IObjPtr src1, uint32 seed){
    hmember* src = (hmember*)src1;
    seed = fasthash32(&src->dt, sizeof(int), seed);
    if(src->onlyType){
        return seed;
    }
    if(dt_is_pointer(src->dt)){
        return dtype_obj_hash(&src->dt, src->value._extra, seed);
    }else{
       DEF_DT_BASE_SWITCH(__HASH_I, src->dt);
    }
    return seed;
}

static void (Func_dump0)(IObjPtr src1, hstring* hs){
    hmember* src = (hmember*)src1;
    hstring_appendf(hs, "dt = %d, onlyType = %d, val = ",
                    src->dt, src->onlyType);
    if(dt_is_pointer(src->dt)){
        dtype_obj_dump(&src->dt, src->value._extra, hs);
    }else{
        DEF_DT_BASE_SWITCH_FORMAT(__DUMP_I, src->dt);
    }
}
static void (Func_ref0)(IObjPtr src1, int c){
    hmember* src = (hmember*)src1;
    if(h_atomic_add(&src->baseObj.ref, c) == -c){
        if(dt_is_pointer(src->dt)){
            dtype_obj_delete(&src->dt, src->value._extra);
        }
        FREE(src);
    }
}
static inline void __hmember_init(hmember* arr){
    IObject_set_name(arr, "__$member");
    arr->baseObj.ref = 1;
    arr->baseObj.Func_copy = Func_copy0;
    arr->baseObj.Func_dump = Func_dump0;
    arr->baseObj.Func_equals = Func_equals0;
    arr->baseObj.Func_hash = Func_hash0;
    arr->baseObj.Func_ref = Func_ref0;
}
//----------------------------
hmember_p hmember_new(int dt){
    hmember_p p = ALLOC(sizeof(hmember));
    __hmember_init(p);
    p->dt = dt;
    p->onlyType = 0;
    return p;
}

#define __SET_I(hffi_t, t)\
case hffi_t:{\
    p->value._##t = ptr->_##t;\
}return kState_OK;

#define __GET_I(hffi_t, t)\
case hffi_t:{\
    ptr->_##t = p->value._##t;\
}return kState_OK;

//int hmember_set_value(hmember_p p,union htype_value* ptr){
//    if(dt_is_pointer(p->dt)){
//        p->value._extra = ptr->_extra;
//        return kState_OK;
//    }else{
//        DEF_DT_BASE_SWITCH(__SET_I, p->dt);
//    }
//    return kState_FAILED;
//}
//int hmember_get_value(hmember_p p,union htype_value* ptr){
//    if(dt_is_pointer(p->dt)){
//        ptr->_extra = p->value._extra;
//        return kState_OK;
//    }else{
//        DEF_DT_BASE_SWITCH(__GET_I, p->dt);
//    }
//    return kState_FAILED;
//}
