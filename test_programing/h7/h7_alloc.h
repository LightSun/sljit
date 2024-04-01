#pragma once

#include <stdlib.h>
#include "h7/common/c_common.h"

//later align memory
#define H7_NEW(s) malloc(s)
#define H7_NEW_COUNT_TYPE(t, c) malloc(sizeof(t) * c)
#define H7_DELETE(ptr) free(ptr)
#define H7_NEW_TYPE(T) (T*)malloc(sizeof(T))

#define H7_NEW_OBJ(T) new T()
#define H7_NEW_OBJ1(T, p) new T(p)
#define H7_DELETE_OBJ(p) delete p

#define H7_REALLOC(ptr, oldS, newS) realloc(ptr, newS)

//2^N
#define kroundup64(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, (x)|=(x)>>32, ++(x))
//2^N
#define kroundup8(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, ++(x))
#define k8N(x) ((x + 8 - 1) & (~(8 - 1)))



//https://blog.csdn.net/qq_43789604/article/details/120385035
/**
  @brief: 向上对齐算法
  @param n：需要对齐数据
  @param align：对齐位数，可选{2，4，6，8，......}
  @retval (n / align + 1) * align：对齐后数据
  */
static inline unsigned int calc_align_up_num(unsigned int n,unsigned align)
{
    /* 当n为align的整数倍 返回n */
    if ( n / align * align == n)
    return n;

    /* 当n不是align的整数倍 返回>n,且离n最近的align的倍数 */
    return  (n / align + 1) * align;
}

/**
  @brief: 向上对齐算法
  @param n：需要对齐数据
  @param align：对齐位数，可选{2，4，6，8，......}
  @retval ((n + align - 1) & (~(align - 1)))：对齐后数据
  */
static inline unsigned int calc_align_up_bit(unsigned int n,unsigned align)
{
    /* 返回n+7，并清除最低三位 */
    return ((n + align - 1) & (~(align - 1)));
}


struct _MemoryBlock{
    void* data;
    uint32 size;
    uint32 allocSize;
};
