#include "sljitLir.h"

#include <stdio.h>
#include <stdlib.h>

struct point_st {
	long x;
	int y;
	short z;
	char d;
    float f;
    const char* str;
};

typedef long (SLJIT_FUNC *point_func_t)(struct point_st *point);;

static long SLJIT_FUNC print_num(long a)
{
	printf("a = %ld\n", a);
	return a + 1;
}

static long SLJIT_FUNC print_float(float a)
{
    printf("a = %.7f\n", a);
    return 0;
}

static long SLJIT_FUNC print_double(double a)
{
    printf("a = %g\n", a);
    return 0;
}

#define REGISTER_SP SLJIT_S0
#define REGISTER_LOCALVAR SLJIT_S1
#define REGISTER_IP SLJIT_S2

void _gen_stack_size_modify(struct sljit_compiler *C, sljit_s32 offset) {
    //sp += offset ;
    sljit_emit_op2(C, SLJIT_ADD, REGISTER_SP, 0,
                   REGISTER_SP, 0,
                   SLJIT_IMM, sizeof(struct point_st) * offset);
}
void _gen_stack_peek_float(struct sljit_compiler *C, sljit_s32 offset,
                           sljit_s32 dst, sljit_sw dstw) {
    //dst=sp[offset]->fvalue
    sljit_emit_fop1(C, SLJIT_MOV_F32, dst, dstw,
                    SLJIT_MEM1(REGISTER_SP),
                    sizeof(struct point_st) * offset
                    + SLJIT_OFFSETOF(struct point_st, f));
}
/*
  This example, we generate a function like this:

long func(struct point_st *point)
{
	print_num(point->x);
	print_num(point->y);
	print_num(point->z);
	print_num(point->d);
	return point->x;
}
*/

static int struct_access2(){
    void *code;
    sljit_uw len;
    point_func_t func;

    struct sljit_compiler *C = sljit_create_compiler(NULL, NULL);

     sljit_emit_enter(C, 0, SLJIT_ARGS1(W, W),
                      1, 1, 1, 0,
                      0);
        //sljit_emit_fcopy 用于float和 double 类型数据转换
     sljit_emit_fset32(C, SLJIT_FR0, 1.234567f);

     sljit_emit_fop1(C, SLJIT_MOV_F32,
                    SLJIT_FR0, 0,
                    SLJIT_IMM, 1.234567f
                    );


    code = sljit_generate_code(C);
    len = sljit_get_generated_code_size(C);

    /* Execute code */
    //func = (point_func_t)code;
    //printf("func return %ld\n", func(&point));

    /* dump_code(code, len); */

    /* Clean up */
    sljit_free_compiler(C);
    sljit_free_code(code, NULL);
}

static int struct_access()
{
	void *code;
	unsigned long len;
	point_func_t func;

	struct point_st point = {
        -5, -20, 5, 'a', 3.1415926f, "heaven7"
	};

	/* Create a SLJIT compiler */
	struct sljit_compiler *C = sljit_create_compiler(NULL, NULL);

    //as point_st: has a float . we need a temp float register.
    sljit_emit_enter(C, 0, SLJIT_ARGS1(W, W), 1, 1, 1, 0, 0);
    /*   opt arg R  S  FR FS local_size */

    // S0->x --> R0
    sljit_emit_op1(C, SLJIT_MOV, SLJIT_R0, 0,
                   SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct point_st, x));	// S0->x --> R0
    // print_num(R0);
    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(W, P),
                     SLJIT_IMM, SLJIT_FUNC_ADDR(print_num));								// print_num(R0);

    // S0->y --> R0
    sljit_emit_op1(C, SLJIT_MOV_S32, SLJIT_R0, 0,
                   SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct point_st, y));	// S0->y --> R0
    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(W, P),
                     SLJIT_IMM, SLJIT_FUNC_ADDR(print_num));								// print_num(R0);

    // S0->z --> R0
    sljit_emit_op1(C, SLJIT_MOV_S16, SLJIT_R0, 0,
                   SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct point_st, z));	// S0->z --> R0
    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(W, P),
                     SLJIT_IMM, SLJIT_FUNC_ADDR(print_num));								// print_num(R0);

    // S0->d --> R0
    sljit_emit_op1(C, SLJIT_MOV_S8, SLJIT_R0, 0,
                   SLJIT_MEM1(SLJIT_S0), SLJIT_OFFSETOF(struct point_st, d));	// S0->d --> R0
    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(W, P),
                     SLJIT_IMM, SLJIT_FUNC_ADDR(print_num));

    // S0->f --> FR0 and print float.
    sljit_emit_fop1(C, SLJIT_MOV_F32, SLJIT_FR0, 0,
                   SLJIT_MEM1(SLJIT_S0),
                   SLJIT_OFFSETOF(struct point_st, f));
    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(W, P),
                     SLJIT_IMM, SLJIT_FUNC_ADDR(print_float));

    // S0->d = 10086
    sljit_emit_op1(C, SLJIT_MOV,
                   SLJIT_MEM1(SLJIT_S0),
                   SLJIT_OFFSETOF(struct point_st, x),
                   SLJIT_IMM, 10086
                   );
    sljit_emit_icall(C, SLJIT_CALL, SLJIT_ARGS1(W, P),
                     SLJIT_IMM, SLJIT_FUNC_ADDR(print_num));
    //return
    sljit_emit_return(C, SLJIT_MOV, SLJIT_MEM1(SLJIT_S0),
                      SLJIT_OFFSETOF(struct point_st, x));				// return S0->x

	/* Generate machine code */
	code = sljit_generate_code(C);
	len = sljit_get_generated_code_size(C);

	/* Execute code */
	func = (point_func_t)code;
	printf("func return %ld\n", func(&point));

	/* dump_code(code, len); */

	/* Clean up */
	sljit_free_compiler(C);
	sljit_free_code(code, NULL);
	return 0;
}

int test_struct()
{
    printf("test_struct >> start ...\n");
	return struct_access();
}
