#include "sljitLir.h"

#include <stdio.h>
#include <stdlib.h>

struct Pt1{
    void* data;
    int size;
};

struct Pt2{
    Pt1 p1;
};

typedef long (SLJIT_FUNC *Func_Pt)(struct Pt2 *point);

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

void test_struct4(){
    void *code;
    unsigned long len;
    Func_Pt func;

    struct Pt1 p1;
    p1.data = malloc(128);
    p1.size = 128;

    ((int*)p1.data)[1] = 100;
    ((int*)p1.data)[2] = 200;

    struct Pt2 pt2;
    pt2.p1 = p1;
    //
    struct sljit_compiler *C = sljit_create_compiler(NULL, NULL);

    //as Pt1: has a float . we need a temp float register.
    sljit_emit_enter(C, 0, SLJIT_ARGS1(W, W),
                     4, 2,
                     2, 0,
                     0);
    //---------------------------------
    //(char*)pt2->data + offset(4)
    printf("data offset = %lld\n", SLJIT_OFFSETOF(struct Pt1, data));
    sljit_emit_op1(C, SLJIT_MOV, SLJIT_R1, 0,
                   SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, data));
    sljit_emit_op1(C, SLJIT_MOV_S32, SLJIT_R0, 0,
                   SLJIT_MEM1(SLJIT_R1), 4);

    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(W, W),
                     SLJIT_IMM, SLJIT_FUNC_ADDR(print_num));
    //---------------------------------
    //return 0.
    sljit_emit_return(C, SLJIT_MOV, SLJIT_IMM, 0);
    //-----------------------------------
    code = sljit_generate_code(C);
    len = sljit_get_generated_code_size(C);

    func = (Func_Pt)code;
    printf("func return %ld\n", func(&pt2));

    /* dump_code(code, len); */

    sljit_free_compiler(C);
    sljit_free_code(code, NULL);
    free(p1.data);
}
