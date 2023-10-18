#include <memory.h>
#include "h_field.h"
#include "h7/h_atomic.h"
#include "h7/hash.h"
#include "h7/h_string.h"
#include "h7/numbers.h"
#include "h7/h_scope.h"

extern void* hobject_get_data_ptr(hobject* p, hfield* src);
static inline uint32 _get_data_size(hfield* src){
    int dt = str2dt(src->dt_desc);
    if(dt_is_base(dt)){
        return dt_size(dt);
    }
    return sizeof(void*);
}

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
     REALLOC2(dst->dt_desc, strlen(src->dt_desc) + 1);
     dst->offset = src->offset;
     if(!dst->owner){
         dst->owner = src->owner;
     }
     //copy data
     if(src->owner){
        void* d_src = hobject_get_data_ptr(src->owner, src);
        void* d_dst = hobject_get_data_ptr(dst->owner, dst);
        if(*(uint64*)d_dst == 0){
            //TODO
        }
        memcpy(d_dst, d_src, _get_data_size(src));
     }
     memcpy(dst->name, src->name, strlen(src->name));
     memcpy(dst->dt_desc, src->dt_desc, strlen(src->dt_desc));
     return dst;
}

static int (Func_equals0)(IObjPtr src1, IObjPtr dst1){
    int ret = IObject_eqauls_base(src1, dst1);
    if(ret != kState_NEXT){
        return ret;
    }
    hfield* src = (hfield*)src1;
    hfield* dst = (hfield*)dst1;
    if(strcmp(src->dt_desc, dst->dt_desc) != 0){
        return kState_FAILED;
    }
//    if(src->owner && dst->owner){
//        int dt = str2dt(src->dt_desc);
//        void* d_src = hobject_get_data_ptr(src->owner, src);
//        void* d_dst = hobject_get_data_ptr(dst->owner, dst);
//        if(dt_is_base(dt)){
//            if(!dtype_base_eq(dt, d_src, d_dst)){
//                return kState_FAILED;
//            }
//        }else{
//           if(!dtype_obj_equals(&dt, d_src, d_dst)){
//               return kState_FAILED;
//           }
//        }
//    }
    return kState_OK;
}

static uint32 (Func_hash0)(IObjPtr src1, uint32 seed){
    hfield* src = (hfield*)src1;
    seed = fasthash32(src->dt_desc, (uint32)strlen(src->dt_desc), seed);
//    if(src->owner){
//        void* d_src = hobject_get_data_ptr(src->owner, src);
//    }
    return seed;
}

static void (Func_dump0)(IObjPtr src1, hstring* hs){
    hfield* src = (hfield*)src1;
    //hstring_appendf(hs, "[field]: dt = %s, onlyType = %d, val = ",
    //                dt2str(src->dt), src->onlyType);
    if(src->owner){
        int dt = str2dt(src->dt_desc);
        void* d_src = hobject_get_data_ptr(src->owner, src);
        dtype_common_dump(dt, d_src, hs);
    }
}
static void (Func_ref0)(IObjPtr src1, int c){
    hfield* src = (hfield*)src1;
    if(h_atomic_add(&src->baseObj.ref, c) == -c){
        FREE(src->dt_desc);
        FREE(src->name);
        FREE(src);
    }
}

DEF_IOBJ_INIT_CHILD(hfield, "$Field")

//----------------------------
hfield_p hfield_new(const char* dt_desc, const char* name){
    hfield_p p = ALLOC(sizeof(hfield));
    __hfield_init(p);
    p->offset = (uint32)DT_OFFSET_NONE;
   // p->size = 0;
    p->dt_desc = hstrdup(dt_desc);
    p->name = name ? hstrdup(name) : NULL;
    p->owner = NULL;
    return p;
}
