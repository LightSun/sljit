#ifndef UTF8_STRING_H
#define UTF8_STRING_H

#include "h7/common/dtypes.h"

typedef unsigned char utf8_char;

typedef struct Utf8String {
    IObject baseObj;
    utf8_char *data;

    uint32 length;
    uint32 _alloced;
} Utf8String;
DEF_IOBJ_CHILD_FUNCS(Utf8String)

Utf8String *utf8_create(void);

Utf8String *utf8_create_c(char const *str);

Utf8String *utf8_create_part_c(char const *str, int start, int len);

Utf8String *utf8_create_copy(Utf8String *str);

Utf8String *utf8_create_part(Utf8String *str, int start, int len);

static inline void utf8_clear(Utf8String *ustr){
    ustr->length = 0;
}

void utf8_append(Utf8String *a1, Utf8String *a2);

void utf8_append_c(Utf8String *a1, char const *a2);

void utf8_append_part(Utf8String *a1, Utf8String *a2, int start, int len);

void utf8_append_part_c(Utf8String *a1, unsigned char const *a2, int start, int len);

void utf8_append_data(Utf8String *a1, char const *a2, s32 size);

s64 utf8_aton(Utf8String *sp, int n);

void utf8_upcase(Utf8String *a1);

void utf8_append_s64(Utf8String *a1, s64 val, int radix);

void utf8_substring(Utf8String *a1, int start, int end);

char const *utf8_cstr(Utf8String *a1);

int utf8_indexof(Utf8String *a1, Utf8String *a2);

int utf8_indexof_c(Utf8String *a1, char const *a2);

int utf8_indexof_pos(Utf8String *a1, Utf8String *a2, int a1_pos); // find a2 from a1, at a1+pos

int utf8_indexof_pos_c(Utf8String *a1, char const *a2, int a1_pos);

int utf8_last_indexof(Utf8String *a1, Utf8String *a2);

int utf8_last_indexof_c(Utf8String *a1, char const *a2);

int utf8_last_indexof_pos(Utf8String *a1, Utf8String *a2, int a1_rightpos);

int utf8_last_indexof_pos_c(Utf8String *a1, char const *a2, int a1_rightpos);

void utf8_replace(Utf8String *a1, Utf8String *a2, Utf8String *a3);

void utf8_replace_c(Utf8String *a1, char const *a2, char const *a3);

int utf8_equals(Utf8String *a1, Utf8String *a2);

int utf8_equals_c(Utf8String *a1, char const *a2);

void utf8_split_get_part(Utf8String *a1, char const *splitor, int index, Utf8String *result);

static inline utf8_char utf8_char_at(Utf8String *a1, int pos) {
    return a1->data[pos];
}

uint32 _utf8_hashCode(Utf8String *ustr);

int utf8_index_of(Utf8String *ustr, utf8_char data);

void utf8_remove(Utf8String *ustr, int index);

void utf8_remove_range(Utf8String *ustr, int index, int length);

int utf8_pushfront(Utf8String *ustr, utf8_char data);

int utf8_pushback(Utf8String *ustr, utf8_char data);

int utf8_insert(Utf8String *ustr, int index, utf8_char data);

int _utf8_enlarge(Utf8String *ustr);

int _utf8_space_require(Utf8String *ustr, int size);

#endif

