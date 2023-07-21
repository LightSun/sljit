#include <memory.h>
#include "h_field.h"
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

static inline void __hfield_init(hfield* arr);

static IObjPtr (Func_copy0)(IObjPtr src1, IObjPtr dst1){
     hfield* src = (hfield*)src1;
     hfield* dst = (hfield*)dst1;
     if(dst == NULL){
         dst = (hfield_p)ALLOC(sizeof(struct hfield));
         __hfield_init(dst);
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
    hfield* src = (hfield*)src1;
    hfield* dst = (hfield*)dst1;
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
    hfield* src = (hfield*)src1;
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
    hfield* src = (hfield*)src1;
    //hstring_appendf(hs, "[field]: dt = %s, onlyType = %d, val = ",
    //                dt2str(src->dt), src->onlyType);
    if(dt_is_pointer(src->dt)){
        dtype_obj_dump(&src->dt, src->value._extra, hs);
    }else{
        DEF_DT_BASE_SWITCH_FORMAT(__DUMP_I, src->dt);
    }
}
static void (Func_ref0)(IObjPtr src1, int c){
    hfield* src = (hfield*)src1;
    if(h_atomic_add(&src->baseObj.ref, c) == -c){
        if(dt_is_pointer(src->dt)){
            dtype_obj_delete(&src->dt, src->value._extra);
        }
        if(src->name){
            FREE(src->name);
        }
        FREE(src);
    }
}

DEF_IOBJ_INIT_CHILD(hfield, "__$field")

//----------------------------
hfield_p hfield_new(const char* name, int dt){
    hfield_p p = ALLOC(sizeof(hfield));
    __hfield_init(p);
    p->dt = dt;
    p->flags = 0;
    p->onlyType = 0;
    p->name = name ? hstrdup(name) : NULL;
    p->value._sint64 = 0;
    return p;
}

#define __SET(hffi_t, t)\
case hffi_t:{\
    p->value._##t = *(t*)ptr;\
}return kState_OK;

int hfield_set(hfield_p p, void* ptr){
    if(dt_is_pointer(p->dt)){
        p->value._extra = ptr;
    }else{
        DEF_DT_BASE_SWITCH(__SET, p->dt);
    }
    return kState_OK;
}

#define __SET_I(hffi_t, t)\
case hffi_t:{\
    p->value._##t = ptr->_##t;\
}return kState_OK;

#define __GET_I(hffi_t, t)\
case hffi_t:{\
    ptr->_##t = p->value._##t;\
}return kState_OK;

//int hfield_set_value(hfield_p p,union htype_value* ptr){
//    if(dt_is_pointer(p->dt)){
//        p->value._extra = ptr->_extra;
//        return kState_OK;
//    }else{
//        DEF_DT_BASE_SWITCH(__SET_I, p->dt);
//    }
//    return kState_FAILED;
//}
//int hfield_get_value(hfield_p p,union htype_value* ptr){
//    if(dt_is_pointer(p->dt)){
//        ptr->_extra = p->value._extra;
//        return kState_OK;
//    }else{
//        DEF_DT_BASE_SWITCH(__GET_I, p->dt);
//    }
//    return kState_FAILED;
//}
