#include "sljitLir.h"

#include <stdio.h>
#include <stdlib.h>

struct Pt1{
    int age;
    float val;
};

struct Pt2{
    int flag;
    char i8;
    Pt1* pt;
};

typedef long (SLJIT_FUNC *Func_Pt)(struct Pt2 *point);;

static long SLJIT_FUNC print_num(long a)
{
    printf("print_num >> a = %ld\n", a);
    return a + 1;
}
static long SLJIT_FUNC print_float(float a)
{
    printf("a = %.7f\n", a);
    return 0;
}

static long SLJIT_FUNC print_float2(float a, long v)
{
    printf("print_float2 >> a(float) = %.3f, v(long) = %ld\n", a, v);
    return 0;
}

void test_struct2()
{
    void *code;
    unsigned long len;
    Func_Pt func;

    struct Pt1 point = {
        20, 1.225f
    };
    struct Pt2 p2;
    p2.flag = 100;
    p2.i8 = 120;
    p2.pt = &point;

    struct sljit_compiler *C = sljit_create_compiler(NULL, NULL);

    //as Pt1: has a float . we need a temp float register.
    sljit_emit_enter(C, 0, SLJIT_ARGS1(W, W),
                     2, 2,
                     2, 0,
                     0);
    //-----------------------------------
    // S0->pt --> R1
    sljit_emit_op1(C, SLJIT_MOV, SLJIT_R1, 0,
                   SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt2, pt));

    // R1->val --> FR0
    sljit_emit_fop1(C, SLJIT_MOV_F32, SLJIT_FR0, 0,
                   SLJIT_MEM1(SLJIT_R1), SLJIT_OFFSETOF(struct Pt1, val));
    //S0->i8 -> R0
    sljit_emit_op1(C, SLJIT_MOV_S8, SLJIT_R0, 0,
                   SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt2, i8));
    //after call function. scratch(tmp) register will be clear.

    //---- call 'print_float()'. ok
    //sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(W, F32),
    //                 SLJIT_IMM, SLJIT_FUNC_ADDR(print_float));
     //---- call 'print_float2()'. ok
    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS2(W, F32,W),
                     SLJIT_IMM, SLJIT_FUNC_ADDR(print_float2));

    //return 0.
    sljit_emit_return(C, SLJIT_MOV, SLJIT_IMM, 0);
    //-----------------------------------
    code = sljit_generate_code(C);
    len = sljit_get_generated_code_size(C);

    func = (Func_Pt)code;
    printf("func return %ld\n", func(&p2));

    /* dump_code(code, len); */

    sljit_free_compiler(C);
    sljit_free_code(code, NULL);
}
