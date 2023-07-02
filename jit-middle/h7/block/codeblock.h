#ifndef h_CODE_BLOCK_H
#define h_CODE_BLOCK_H

#include "hstatement.h"
#include "sarray.h"

// if .else .
//switch
//for
//while

struct sljit_jump;

typedef struct codeblock_if{
    struct sljit_jump *cond;
    struct sljit_jump *out;

    sa_hstatement if_sts;
    sa_hstatement else_if_sts; //sarray<sarray>
    sa_hstatement else_sts;
}codeblock_if;

codeblock_if* codeblock_if_new();

#endif

