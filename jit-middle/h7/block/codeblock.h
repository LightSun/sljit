#ifndef h_CODE_BLOCK_H
#define h_CODE_BLOCK_H

#include "hstatement.h"
#include "sarray.h"

// if .else .
//switch
//for
//while

struct sljit_jump;
struct HashTable;

typedef struct codeblock_if{
    struct sljit_jump *cond;
    struct sljit_jump *out;

    sa_hstatement if_sts;
    sa_hstatement else_if_sts; //sarray<sarray>
    sa_hstatement else_sts;
}codeblock_if;

typedef struct codeblock_while{
    struct sljit_jump *cond;
    struct sljit_jump *out;

    sa_hstatement conds;
    sa_hstatement mains;
}codeblock_while;

typedef struct codeblock_for{
    struct sljit_jump *cond;
    struct sljit_jump *out;

    sa_hstatement inits;
    sa_hstatement conds;
    sa_hstatement posts;
}codeblock_for;

typedef struct codeblock_switch{
    struct sljit_jump *cond;
    struct sljit_jump *out;

    struct HashTable* kvs;
}codeblock_switch;

codeblock_if* codeblock_if_new();

#endif

