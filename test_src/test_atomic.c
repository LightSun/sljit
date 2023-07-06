#include "sljitLir.h"
#include <stdio.h>

#define T 16

#if T == 16
typedef sljit_u16 test_t;
#define M SLJIT_MOV_U16
#define D 0x1337
#else
typedef sljit_u8 test_t;
#define M SLJIT_MOV_U8
#define D 0x42
#endif

typedef long (SLJIT_FUNC *func_t)(test_t *);

static long SLJIT_FUNC print_num(long a)
{
    printf("%ld\n", a);
    return a + 1;
}

static func_t generate_func(void)
{
    void *code;

    struct sljit_compiler *C = sljit_create_compiler(NULL, NULL);

    sljit_emit_enter(C, 0, SLJIT_ARGS1(W, P), 2, 1, 0, 0,
                     0);
    //load from SLJIT_S0 -> SLJIT_R0
    sljit_emit_atomic_load(C, M, SLJIT_R0, SLJIT_S0);
    sljit_emit_op1(C, M, SLJIT_R1, 0, SLJIT_IMM, D);
    //save from SLJIT_R1 -> SLJIT_S0
    sljit_emit_atomic_store(C, M, SLJIT_R1, SLJIT_S0, SLJIT_R0);

    sljit_emit_return(C, SLJIT_MOV, SLJIT_R0, 0);

    code = sljit_generate_code(C);

    sljit_free_compiler(C);
    return (func_t)code;

}

/** func_ret arr[i] raw
0: beef beef 5566aabbdeadbeef
1: dead dead 5566aabbdeadbeef
2: aabb aabb 5566aabbdeadbeef
3: 5566 5566 5566aabbdeadbeef
 */
int test_atomic2(void)
{
    sljit_uw c = 0x5566aabbdeadbeef;
    func_t func = generate_func();

    //%lx，表示long类型整数，用16进制输出；
    for (int i = 0; i < (int)(sizeof(c)/sizeof(test_t)); i++) {
        printf("%d: %lx %hx %lx\n", i,
               func((test_t *)&c + i),
               ((test_t *)&c)[i], c);
        c = 0x5566aabbdeadbeef;
    }

    sljit_free_code((void *)func, NULL);
    return 0;
}
