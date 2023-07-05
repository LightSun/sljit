
#include "sljitLir.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef long (SLJIT_FUNC *Func)();

static long SLJIT_FUNC print_num(long a)
{
    printf("%ld\n", a);
    return a + 1;
}
static long SLJIT_FUNC print_num2(long a, long b)
{
    printf("a, b (%ld, %ld)\n", a, b);
    return 0;
}

#define HEMIT(op, num1, num2)\
sljit_emit_op2(C, op, SLJIT_R0, 0, SLJIT_IMM, num1, SLJIT_IMM, num2);\
printf("%s: %d, %d = \n", #op, num1, num2);\
/*//异或 return W = word(64 bit int), param p = pointer is the function addr.*/\
sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(W, P),\
                 SLJIT_IMM, SLJIT_FUNC_ADDR(print_num));

#define HEMIT1(op, num1)\
sljit_emit_op1(C, op, SLJIT_R0, 0, SLJIT_IMM, num1);\
printf("%s: %d = \n", #op, num1);\
sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(W, P),\
                 SLJIT_IMM, SLJIT_FUNC_ADDR(print_num));

//  R0 = R0 op R1.  op can be '/' '*' and etc.
#define HEMIT_OP0_INT(op, r1_v, r0_v)\
    sljit_emit_op1(C, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, r0_v);\
    sljit_emit_op1(C, SLJIT_MOV, SLJIT_R1, 0, SLJIT_IMM, r1_v);\
    printf("op0 >> %s: %d, %d = \n", #op, r1_v, r0_v);\
    sljit_emit_op0(C, op);\
    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS2(W, W, P),\
                     SLJIT_IMM, SLJIT_FUNC_ADDR(print_num2));

#define HEMIT_OP0_INT2(op, r1_v, r0_v, code)\
    sljit_emit_op1(C, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, r0_v);\
    sljit_emit_op1(C, SLJIT_MOV, SLJIT_R1, 0, SLJIT_IMM, r1_v);\
    printf("op0 >> %s: %d, %d = \n", #op, r1_v, r0_v);\
    sljit_emit_op0(C, op);\
    code;\
    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS2(W, W, P),\
                     SLJIT_IMM, SLJIT_FUNC_ADDR(print_num2));

static void test_sljit_st1();

void test_sljit_st(){
    test_sljit_st1();
}

void test_sljit_st1(){

    void *code;
    sljit_uw len;
    struct sljit_compiler *C = sljit_create_compiler(NULL, NULL);

    /* 3 arg, 1 temp reg, 3 save reg */
    sljit_emit_enter(C, 0, SLJIT_ARGS0(W),
                     4, 0, 2, 0,
                     0);
    //* R0 = a & 1, S0 is argument a */
   // sljit_emit_op2(C, SLJIT_AND, SLJIT_R0, 0, SLJIT_S0, 0, SLJIT_IMM, 1); //

    //101 vs 110.  011
    sljit_emit_op2(C, SLJIT_XOR, SLJIT_R0, 0, SLJIT_IMM, 5, SLJIT_IMM, 6); //异或
    printf("SLJIT_XOR: 5, 6 = \n");
    //return W = word(64 bit int), param p = pointer is the function addr.
    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(W, P),
                     SLJIT_IMM, SLJIT_FUNC_ADDR(print_num));
    const char* val= "2";
    HEMIT(SLJIT_ADD, 8, atoi(val));
    HEMIT(SLJIT_ADD, 8, 2);
    HEMIT(SLJIT_SUB, 8, 2);
    HEMIT(SLJIT_MUL, 8, 2);
    HEMIT(SLJIT_OR, 5, 6); //7

    HEMIT(SLJIT_SHL, 1, 8); //  1 << 8.
    HEMIT(SLJIT_MSHL, 1, 8);

    HEMIT(SLJIT_LSHR, 8, 2); // 8 >> 2
    HEMIT(SLJIT_MLSHR, 8, 2);
    HEMIT(SLJIT_ASHR, 8, 3);

    HEMIT(SLJIT_ROTL, 3, 8); // 3 * (1 << 8)
    HEMIT(SLJIT_ROTR, 32, 3); // 32 / (1 << 3)
    //
    HEMIT_OP0_INT(SLJIT_DIV_SW, 3, 9); // no remainder. result in R0. R1 not changed.

    //used to mod
    HEMIT_OP0_INT2(SLJIT_DIVMOD_SW, 3, 10, {
                 sljit_emit_op1(C, SLJIT_MOV, SLJIT_R0, 0, SLJIT_R1, 0); // R0, R1: 3, 1 -> 1, 1
                   }); //remainder 1 in R1. result in R0,
    //
    //sljit_emit_atomic_load() / sljit_emit_atomic_store()

    //u8 max = 255
    HEMIT1(SLJIT_MOV_U8, 256);
    //反转字节序.
    // 0000 0000 0000 1101 -> 0000 1101 0000 0000
    HEMIT1(SLJIT_REV_U16, 13);
    //
    sljit_emit_return(C, SLJIT_MOV, SLJIT_IMM, 0);

    code = sljit_generate_code(C);
    len = sljit_get_generated_code_size(C);

    Func func = (Func)code;
    func();

    sljit_free_compiler(C);
    sljit_free_code(code, NULL);
}
