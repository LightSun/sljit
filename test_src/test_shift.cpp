#include "test_h.h"

static bool verbose = true;

//test9()
void test_shift()
{
    /* Test shift. */
    executable_code code;
    struct sljit_compiler* compiler = sljit_create_compiler(NULL, NULL);
    auto C = compiler;
    sljit_sw buf[15];
    sljit_s32 i;
#ifdef SLJIT_PREF_SHIFT_REG
    sljit_s32 shift_reg = SLJIT_PREF_SHIFT_REG;
#else
    sljit_s32 shift_reg = SLJIT_R2;
#endif

    SLJIT_ASSERT(shift_reg >= SLJIT_R2 && shift_reg <= SLJIT_R3);

    if (verbose)
        printf("Run test9\n");

    FAILED(!compiler, "cannot create compiler\n");

    for (i = 0; i < 15; i++)
        buf[i] = -1;

    buf[4] = 1 << 10;
    buf[9] = 3;

    //SLJIT_S0 is data-pointer
    sljit_emit_enter(compiler, 0, SLJIT_ARGS1(VOID, P), 4, 4, 0, 0, 0);
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, 0xf);
    //SLJIT_MSHL 类似SLJIT_SHL, 具体区别还不清楚
    sljit_emit_op2(compiler, SLJIT_SHL, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_IMM, 3);

    _PRINT("SLJIT_SHL: 0xf << 3", SLJIT_R0); //120

    sljit_emit_op2(compiler, SLJIT_LSHR, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_IMM, 1);
    _PRINT("SLJIT_LSHR: 120 >> 1", SLJIT_R0); //60

    //---- for not change below-flow---
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, 60);
    /* buf[0] */
    //buf[0] = 60
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_MEM1(SLJIT_S0), 0, SLJIT_R0, 0);

    sljit_emit_op2(compiler, SLJIT_SHL, SLJIT_R1, 0, SLJIT_R0, 0, SLJIT_IMM, 1);//R1=60 << 1
    /* buf[1] */
    sljit_emit_op2(compiler, SLJIT_SHL, SLJIT_MEM1(SLJIT_S0),
                   sizeof(sljit_sw), SLJIT_R1, 0, SLJIT_IMM, 1);
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, -64);
    sljit_emit_op1(compiler, SLJIT_MOV, shift_reg, 0, SLJIT_IMM, 2);
    /* buf[2] */
    //SLJIT_ASHR: 带符号右移
    sljit_emit_op2(compiler, SLJIT_ASHR, SLJIT_MEM1(SLJIT_S0), sizeof(sljit_sw) * 2,
                   SLJIT_R0, 0, shift_reg, 0);

    sljit_emit_op1(compiler, SLJIT_MOV, shift_reg, 0, SLJIT_IMM, 0xff);
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, 4);
    sljit_emit_op2(compiler, SLJIT_SHL, shift_reg, 0, shift_reg, 0, SLJIT_R0, 0);
    /* buf[3] */
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_MEM1(SLJIT_S0), sizeof(sljit_sw) * 3, shift_reg, 0);
    sljit_emit_op1(compiler, SLJIT_MOV, shift_reg, 0, SLJIT_IMM, 0xff);
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, 8);
    /* buf[4] */
    sljit_emit_op2(compiler, SLJIT_LSHR, SLJIT_MEM1(SLJIT_S0), sizeof(sljit_sw) * 4, SLJIT_MEM1(SLJIT_S0), sizeof(sljit_sw) * 4, SLJIT_R0, 0);
    /* buf[5] */
    sljit_emit_op2(compiler, SLJIT_SHL, SLJIT_MEM1(SLJIT_S0), sizeof(sljit_sw) * 5, shift_reg, 0, SLJIT_R0, 0);

    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_S1, 0, SLJIT_IMM, 0xf);
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, 2);
    sljit_emit_op2(compiler, SLJIT_SHL, SLJIT_S1, 0, SLJIT_S1, 0, SLJIT_R0, 0);
    /* buf[6] */
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_MEM1(SLJIT_S0), sizeof(sljit_sw) * 6, SLJIT_S1, 0);
    sljit_emit_op2(compiler, SLJIT_SHL, SLJIT_R0, 0, SLJIT_S1, 0, SLJIT_R0, 0);
    /* buf[7] */
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_MEM1(SLJIT_S0), sizeof(sljit_sw) * 7, SLJIT_R0, 0);
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R2, 0, SLJIT_IMM, 0xf00);
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, 4);
    sljit_emit_op2(compiler, SLJIT_LSHR, SLJIT_R1, 0, SLJIT_R2, 0, SLJIT_R0, 0);
    /* buf[8] */
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_MEM1(SLJIT_S0), sizeof(sljit_sw) * 8, SLJIT_R1, 0);

    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, (sljit_sw)buf);
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R1, 0, SLJIT_IMM, 9);
    /* buf[9] */
    sljit_emit_op2(compiler, SLJIT_SHL, SLJIT_MEM2(SLJIT_R0, SLJIT_R1), SLJIT_WORD_SHIFT, SLJIT_MEM2(SLJIT_R0, SLJIT_R1), SLJIT_WORD_SHIFT, SLJIT_MEM2(SLJIT_R0, SLJIT_R1), SLJIT_WORD_SHIFT);

    sljit_emit_op1(compiler, SLJIT_MOV, shift_reg, 0, SLJIT_IMM, 4);
    sljit_emit_op2(compiler, SLJIT_SHL, shift_reg, 0, SLJIT_IMM, 2, shift_reg, 0);
    /* buf[10] */
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_MEM1(SLJIT_S0), sizeof(sljit_sw) * 10, shift_reg, 0);

    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, 0xa9);
    sljit_emit_op2(compiler, SLJIT_SHL, SLJIT_R1, 0, SLJIT_R0, 0, SLJIT_IMM, 0);
    sljit_emit_op1(compiler, SLJIT_MOV32, SLJIT_R0, 0, SLJIT_IMM, 0x7d00);
    sljit_emit_op2(compiler, SLJIT_LSHR32, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_IMM, 32);
#if (defined SLJIT_64BIT_ARCHITECTURE && SLJIT_64BIT_ARCHITECTURE)
    sljit_emit_op1(compiler, SLJIT_MOV_U32, SLJIT_R0, 0, SLJIT_R0, 0);
#endif
    sljit_emit_op2(compiler, SLJIT_OR, SLJIT_R1, 0, SLJIT_R1, 0, SLJIT_R0, 0);
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, 0xe30000);
#if (defined SLJIT_64BIT_ARCHITECTURE && SLJIT_64BIT_ARCHITECTURE)
    sljit_emit_op2(compiler, SLJIT_ASHR, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_IMM, 0xffc0);
#else
    sljit_emit_op2(compiler, SLJIT_ASHR, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_IMM, 0xffe0);
#endif
    sljit_emit_op2(compiler, SLJIT_OR, SLJIT_R1, 0, SLJIT_R1, 0, SLJIT_R0, 0);
    sljit_emit_op1(compiler, SLJIT_MOV32, SLJIT_R0, 0, SLJIT_IMM, 0x25000000);
    sljit_emit_op2(compiler, SLJIT_SHL32, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_IMM, 0xfffe1);
#if (defined SLJIT_64BIT_ARCHITECTURE && SLJIT_64BIT_ARCHITECTURE)
    sljit_emit_op1(compiler, SLJIT_MOV_U32, SLJIT_R0, 0, SLJIT_R0, 0);
#endif
    /* buf[11] */
    sljit_emit_op2(compiler, SLJIT_OR, SLJIT_MEM1(SLJIT_S0), sizeof(sljit_sw) * 11, SLJIT_R1, 0, SLJIT_R0, 0);

    sljit_emit_op1(compiler, SLJIT_MOV, shift_reg, 0, SLJIT_IMM, 0);
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, 0x5c);
    sljit_emit_op2(compiler, SLJIT_SHL, SLJIT_R1, 0, SLJIT_R0, 0, shift_reg, 0);
    sljit_emit_op1(compiler, SLJIT_MOV32, SLJIT_R0, 0, SLJIT_IMM, 0xf600);
    sljit_emit_op2(compiler, SLJIT_LSHR32, SLJIT_R0, 0, SLJIT_R0, 0, shift_reg, 0);
#if (defined SLJIT_64BIT_ARCHITECTURE && SLJIT_64BIT_ARCHITECTURE)
    /* Alternative form of uint32 type cast. */
    sljit_emit_op2(compiler, SLJIT_AND, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_IMM, 0xffffffff);
#endif
    sljit_emit_op2(compiler, SLJIT_OR, SLJIT_R1, 0, SLJIT_R1, 0, SLJIT_R0, 0);
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, 0x630000);
    sljit_emit_op2(compiler, SLJIT_ASHR, SLJIT_R0, 0, SLJIT_R0, 0, shift_reg, 0);
    /* buf[12] */
    sljit_emit_op2(compiler, SLJIT_OR, SLJIT_MEM1(SLJIT_S0), sizeof(sljit_sw) * 12, SLJIT_R1, 0, SLJIT_R0, 0);

    /* Test shift_reg keeps 64 bit value after 32 bit operation. */
    sljit_emit_op1(compiler, SLJIT_MOV32, SLJIT_R0, 0, SLJIT_IMM, 0);
    sljit_emit_op1(compiler, SLJIT_MOV32, SLJIT_R1, 0, SLJIT_IMM, 0);
    sljit_emit_op1(compiler, SLJIT_MOV, shift_reg, 0, SLJIT_IMM, -3062);
    sljit_emit_op2(compiler, SLJIT_ASHR32, SLJIT_R0, 0, SLJIT_R0, 0, SLJIT_R1, 0);
    /* buf[13] */
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_MEM1(SLJIT_S0), sizeof(sljit_sw) * 13, shift_reg, 0);

    sljit_emit_op1(compiler, SLJIT_MOV, shift_reg, 0, SLJIT_IMM, -4691);
    sljit_emit_op2(compiler, SLJIT_LSHR32, SLJIT_R0, 0, SLJIT_R1, 0, SLJIT_R0, 0);
    /* buf[14] */
    sljit_emit_op1(compiler, SLJIT_MOV, SLJIT_MEM1(SLJIT_S0), sizeof(sljit_sw) * 14, shift_reg, 0);

    sljit_emit_return_void(compiler);

    code.code = sljit_generate_code(compiler);
    CHECK(compiler);
    sljit_free_compiler(compiler);

    code.func1((sljit_sw)&buf);
    for(int i = 0 ; i < 15 ; ++i){
        printf("buf[%d] = %ld\n", i, buf[i]);
    }
    FAILED(buf[0] != 0x3c, "test9 case 1 failed\n");
    FAILED(buf[1] != 0xf0, "test9 case 2 failed\n");
    FAILED(buf[2] != -16, "test9 case 3 failed\n");
    FAILED(buf[3] != 0xff0, "test9 case 4 failed\n");
    FAILED(buf[4] != 4, "test9 case 5 failed\n");
    FAILED(buf[5] != 0xff00, "test9 case 6 failed\n");
    FAILED(buf[6] != 0x3c, "test9 case 7 failed\n");
    FAILED(buf[7] != 0xf0, "test9 case 8 failed\n");
    FAILED(buf[8] != 0xf0, "test9 case 9 failed\n");
    FAILED(buf[9] != 0x18, "test9 case 10 failed\n");
    FAILED(buf[10] != 32, "test9 case 11 failed\n");
    FAILED(buf[11] != 0x4ae37da9, "test9 case 12 failed\n");
    FAILED(buf[12] != 0x63f65c, "test9 case 13 failed\n");
    FAILED(buf[13] != -3062, "test9 case 14 failed\n");
    FAILED(buf[14] != -4691, "test9 case 15 failed\n");

    sljit_free_code(code.code, NULL);
}
