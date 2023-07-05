#include "h_method.h"

#include "h7/common/halloc.h"
#include "h7/h_atomic.h"
#include "h7/hash.h"
#include "h7/h_string.h"

static inline void __hmethod_init(hmethod* arr);

static IObjPtr (Func_copy0)(IObjPtr src1, IObjPtr dst1){
     hmethod* src = (hmethod*)src1;
     hmethod* dst = (hmethod*)dst1;
     if(dst == NULL){
        //src->ret
        dst = hmethod_new(src->name, hfield_copy(src->ret),
                          harray_copy(src->params));
     }else{
        hfield_unref(dst->ret);
        harray_unref(dst->params);
        dst->ret = hfield_copy(src->ret);
        dst->params = harray_copy(src->params);
     }
     return dst;
}
static int (Func_equals0)(IObjPtr src1, IObjPtr dst1){
    int ret = IObject_eqauls_base(src1, dst1);
    if(ret != kState_NEXT){
        return ret;
    }
    hmethod* src = (hmethod*)src1;
    hmethod* dst = (hmethod*)dst1;
    if(!hfield_equals(src->ret, dst->ret)){
        return kState_FAILED;
    }
    if(!harray_equals(src->params, dst->params)){
        return kState_FAILED;
    }
    return kState_OK;
}
static uint32 (Func_hash0)(IObjPtr src1, uint32 seed){
    hmethod* src = (hmethod*)src1;
    seed = hfield_hash(src->ret, seed);
    return harray_hash(src->params, seed);
}
static void (Func_dump0)(IObjPtr src1, hstring* hs){
    hmethod* src = (hmethod*)src1;
    hstring_append(hs, "ret = ");
    hfield_dump(src->ret, hs);
    hstring_append(hs, ", params = ");
    harray_dump(src->params, hs);
}
static void (Func_ref0)(IObjPtr src1, int c){
    hmethod* src = (hmethod*)src1;
    if(h_atomic_add(&src->baseObj.ref, c) == -c){
        hfield_unref(src->ret);
        harray_unref(src->params);
        FREE(src->name);
        FREE(src);
    }
}

DEF_IOBJ_INIT_CHILD(hmethod, "__$method")

//---------------------------------------------------
hmethod_p hmethod_new(const char* name, hfield_p ret, harray_p params){
    hmethod_p p = ALLOC_T(hmethod);
    __hmethod_init(p);
    p->name = hstrdup(name);
    p->ret = ret;
    p->params = params;
    return p;
}
