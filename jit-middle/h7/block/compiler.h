#ifndef H_COMPILER_H
#define H_COMPILER_H

#include "h7/common/c_common.h"
#include "h7/block/sarray.h"

struct sljit_compiler;

typedef struct hcompiler_contxt{
    struct sljit_compiler* C;

}hcompiler_contxt, *hcompiler_contxt_p;

int hcompiler_compile(char* buf, size_t len, hcompiler_contxt_p ctx);

#endif



