#pragma once

#include "sljitLir.h"

#include <stdio.h>
#include <stdlib.h>

#define FAILED(cond, text) \
    if (SLJIT_UNLIKELY(cond)) { \
        printf(text); \
        return; \
    }

#define CHECK(compiler) \
    if (sljit_get_compiler_error(compiler) != SLJIT_ERR_COMPILED) { \
        printf("Compiler error: %d\n", sljit_get_compiler_error(compiler)); \
        sljit_free_compiler(compiler); \
        return; \
    }

union executable_code {
    void* code;
    sljit_sw (SLJIT_FUNC *func0)(void);
    sljit_sw (SLJIT_FUNC *func1)(sljit_sw a);
    sljit_sw (SLJIT_FUNC *func2)(sljit_sw a, sljit_sw b);
    sljit_sw (SLJIT_FUNC *func3)(sljit_sw a, sljit_sw b, sljit_sw c);

    void (SLJIT_FUNC *test70_f1)(sljit_s32 a, sljit_uw b, sljit_u32 c, sljit_sw d);
    void (SLJIT_FUNC *test70_f2)(sljit_s32 a, sljit_u32 b, sljit_sw c, sljit_sw d);
    void (SLJIT_FUNC *test70_f3)(sljit_s32 a, sljit_f32 b, sljit_uw c, sljit_f64 d);
    void (SLJIT_FUNC *test70_f4)(sljit_f32 a, sljit_f64 b, sljit_f32 c, sljit_s32 d);
    void (SLJIT_FUNC *test70_f5)(sljit_f64 a, sljit_f32 b, sljit_u32 c, sljit_f32 d);
    void (SLJIT_FUNC *test70_f6)(sljit_f64 a, sljit_s32 b, sljit_f32 c, sljit_f64 d);
    void (SLJIT_FUNC *test70_f7)(sljit_f32 a, sljit_s32 b, sljit_uw c, sljit_u32 d);
    void (SLJIT_FUNC *test70_f8)(sljit_f64 a, sljit_f64 b, sljit_uw c, sljit_sw d);
    void (SLJIT_FUNC *test70_f9)(sljit_f64 a, sljit_f64 b, sljit_uw c, sljit_f64 d);
    void (SLJIT_FUNC *test70_f10)(sljit_f64 a, sljit_f64 b, sljit_f64 c, sljit_s32 d);

    sljit_sw (SLJIT_FUNC *test71_f1)(sljit_f32 a, sljit_f64 b);
    sljit_sw (SLJIT_FUNC *test71_f2)(sljit_f64 a, sljit_f64 b, sljit_f64 c, sljit_f64 d);
    sljit_sw (SLJIT_FUNC *test71_f3)(sljit_f64 a, sljit_f64 b, sljit_f64 c);

    void (SLJIT_FUNC *test73_f1)(sljit_s32 a, sljit_sw b, sljit_sw c, sljit_s32 d);
    void (SLJIT_FUNC *test73_f2)(sljit_sw a, sljit_sw b, sljit_s32 c, sljit_s32 d);
    void (SLJIT_FUNC *test73_f3)(sljit_f64 a, sljit_f64 b, sljit_f64 c, sljit_sw d);
    void (SLJIT_FUNC *test73_f4)(sljit_f64 a, sljit_f64 b, sljit_sw c, sljit_sw d);

    sljit_f32 (SLJIT_FUNC *test81_f1)(sljit_sw a);
    sljit_f64 (SLJIT_FUNC *test81_f2)(sljit_sw a);
};
typedef union executable_code executable_code;

//----------------
static void SLJIT_FUNC print_num(long a)
{
    printf(" = %ld\n", a);
}
//4 temp, 4 save
//after sljit_emit_icall: all register R0-Rn will be reset to 0.
#define _PRINT(prefix, reg)\
    printf("%s =\n", prefix);\
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_S3, 0, SLJIT_R0, 0);\
    if(reg != SLJIT_R0){\
        sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, reg, 0);\
    }\
    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(VOID, W),\
                 SLJIT_IMM, SLJIT_FUNC_ADDR(print_num));\
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_S3, 0);
