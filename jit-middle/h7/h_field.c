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
         IObject_copy_base_unsafe(kType_P_FIELD, src, dst);
     }else{
         IObject_copy_base(kType_P_FIELD, src, dst);
     }
     REALLOC2(dst->name, strlen(src->name) + 1);
     dst->offset = src->offset;
     return dst;
}

static int (Func_equals0)(IObjPtr src1, IObjPtr dst1){
    int ret = IObject_eqauls_base(src1, dst1);
    if(ret != kState_NEXT){
        return ret;
    }
    hfield* src = (hfield*)src1;
    hfield* dst = (hfield*)dst1;
    if(src->onlyType){
        return kState_OK;
    }
    return kState_OK;
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

DEF_IOBJ_INIT_CHILD(hfield, "$Field")

//----------------------------
hfield_p hfield_new(const char* name){
    hfield_p p = ALLOC(sizeof(hfield));
    __hfield_init(p);
    p->offset = DT_OFFSET_NONE;
    p->name = name ? hstrdup(name) : NULL;
    return p;
}
