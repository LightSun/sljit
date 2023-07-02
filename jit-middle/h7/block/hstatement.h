#ifndef h_STATEMENT_H
#define h_STATEMENT_H


#include <stdlib.h>
#include "sarray.h"

typedef enum hst_op{
    ADD = 1, DEC, MUL, DIV, MOD,
    OPN, // -10, -t
    NOT, AND_AND, EQ_EQ, NEQ, OR_OR,  //!, &&,==, !=, ||
    GT,GE,LT,LE,
    AS,IS,NIS,      //as, is, not is
    AND, OR, NOR, BRE,LSHIFT,RSHIFT, // &,|,^,~,<<, >>
    EQ,               //equal
    ADD_EQ, DEC_EQ, MUL_EQ, DIV_EQ, MOD_EQ,
    LSHIFT_EQ, RSHIFT_EQ,
    OR_EQ,   // |=,
    AND_EQ,  // &=
}hst_op;

typedef struct hstatement{
    int op_c;  //sljit_emit_op2(C, SLJIT_AND, SLJIT_R0, 0, SLJIT_S0, 0, SLJIT_IMM, 1); op_c = 2

}hstatement;

SARRAY_INIT(hstatement);

#endif
