#include "sljitLir.h"

#include <stdio.h>
#include <stdlib.h>

struct Pt1{
    int age;
    double d1;
    double d2;
    float f1;
    float f2;

    size_t l;
    char i8;
    unsigned char u8;
};

typedef long (SLJIT_FUNC *Func_Pt)(struct Pt1 *point);

static long SLJIT_FUNC print_double(double a)
{
    printf("print_double >> a = %.3f\n", a);
    return 0;
}

void test_float(){
    void *code;
    unsigned long len;
    Func_Pt func;

    //age,val, val2
    struct Pt1 point = {
        10, -21.55, 22.55, -31.1, 32.1,
        0, -1, 1
    };

    struct sljit_compiler *C = sljit_create_compiler(NULL, NULL);

    //as Pt1: has a float . we need a temp float register.
    sljit_emit_enter(C, 0, SLJIT_ARGS1(W, W),
                     4, 2,
                     4, 0,
                     0);
    //----------------------------
    //d1 = d2. ok
//    sljit_emit_fop1(C, SLJIT_MOV_F64,
//                    SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, d1),
//                    SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, d2));
    //f1 = f2. ok
//    sljit_emit_fop1(C, SLJIT_MOV_F32,
//                    SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, f1),
//                    SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, f2));

    //f32: float to double . nothing changed.
//    sljit_emit_fop1(C, SLJIT_MOV_F32,
//                        SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, d1),
//                        SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, f1));

    //convert float to double. ok
//    sljit_emit_fop1(C, SLJIT_CONV_F64_FROM_F32,
//                            SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, d1),
//                            SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, f1));

    //convert double to float. ok
//    sljit_emit_fop1(C, SLJIT_CONV_F32_FROM_F64,
//                            SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, f1),
//                            SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, d1));

    //u8 to long. ok
//    sljit_emit_op1(C, SLJIT_MOV_U8,
//                            SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, l),
//                            SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, u8));

    //long to i8. ok.
    sljit_emit_op1(C, SLJIT_MOV_S8,
                            SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, i8),
                            SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct Pt1, l));
    //float to double in one reg. ok
    sljit_emit_fset32(C, SLJIT_FR0, 1.2f);
    sljit_emit_fop1(C, SLJIT_CONV_F64_FROM_F32,
                                SLJIT_FR0, 0,
                                SLJIT_FR0, 0);
    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(W, F64),
                     SLJIT_IMM, SLJIT_FUNC_ADDR(print_double));
    //-----------------------------
    //return 0.
    sljit_emit_return(C, SLJIT_MOV, SLJIT_IMM, 0);
    //-----------------------------------
    code = sljit_generate_code(C);
    len = sljit_get_generated_code_size(C);

    func = (Func_Pt)code;
    printf("func return %ld\n", func(&point));

    /* dump_code(code, len); */

    sljit_free_compiler(C);
    sljit_free_code(code, NULL);

    printf("point floats: d1,d2, f1, f2 = (%.3f,%.3f，%.3f，%.3f)\n",
           point.d1, point.d2, point.f1, point.f2);
    printf("point ints: l, i8, u8 = (%ld, %d，%d)\n",
           point.l, point.i8, point.u8);
}
