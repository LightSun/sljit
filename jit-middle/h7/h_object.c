#include "h_object.h"

#include "h7/common/halloc.h"
#include "h7/h_atomic.h"
#include "h7/hash.h"
#include "h7/h_string.h"

static inline void __hobject_init(hobject* arr);

static IObjPtr (Func_copy0)(IObjPtr src1, IObjPtr dst1){
     hobject* src = (hobject*)src1;
     hobject* dst = (hobject*)dst1;
     if(dst == NULL){
         harray* fileds = src->fields ? harray_copy(src->fields) : NULL;
         harray* methods = src->methods ? harray_copy(src->methods) : NULL;
         dst = hobject_new(fileds, methods);
     }else{
         if(dst->fields){
             harray_unref(dst->fields);
         }
         if(dst->methods){
             harray_unref(dst->methods);
         }
         dst->fields = src->fields ? harray_copy(src->fields) : NULL;
         dst->methods = src->methods ? harray_copy(src->methods) : NULL;
     }
     return dst;
}
static int (Func_equals0)(IObjPtr src1, IObjPtr dst1){
    int ret = IObject_eqauls_base(src1, dst1);
    if(ret != kState_NEXT){
        return ret;
    }
    hobject* src = (hobject*)src1;
    hobject* dst = (hobject*)dst1;
    return harray_equals(src->fields, dst->fields);
}
static uint32 (Func_hash0)(IObjPtr src1, uint32 seed){
    hobject* src = (hobject*)src1;
    if(!src->fields){
        return seed;
    }
    return harray_hash(src->fields, seed);
}
static void (Func_dump0)(IObjPtr src1, hstring* hs){
    hobject* src = (hobject*)src1;
    hstring_appendf(hs, "[%s]: fields=" NEW_LINE, src->baseObj.name);
    if(src->fields){
        harray_dump(src->fields, hs);
        hstring_append(hs, NEW_LINE);
    }
    hstring_append(hs, "methods=");
    if(src->methods){
        harray_dump(src->methods, hs);
    }
    hstring_append(hs, NEW_LINE);
}
static void (Func_ref0)(IObjPtr src1, int c){
    hobject* src = (hobject*)src1;
    if(h_atomic_add(&src->baseObj.ref, c) == -c){
        if(src->fields){
            harray_delete(src->fields);
        }
        if(src->methods){
            harray_delete(src->methods);
        }
        FREE(src);
    }
}
static inline void __hobject_init(hobject* arr){
    IObject_set_name(arr, "__$object");
    arr->baseObj.ref = 1;
    arr->baseObj.Func_copy = Func_copy0;
    arr->baseObj.Func_dump = Func_dump0;
    arr->baseObj.Func_equals = Func_equals0;
    arr->baseObj.Func_hash = Func_hash0;
    arr->baseObj.Func_ref = Func_ref0;
}

//-----------------------------------------------------
hobject* hobject_new(harray_p fields, harray_p methods){
    hobject_p p = ALLOC_T(hobject);
    __hobject_init(p);
    p->fields = fields;
    p->methods = methods;
    return p;
}
