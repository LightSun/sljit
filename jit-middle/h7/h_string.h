#ifndef H_STRING_H
#define H_STRING_H

#include <string.h>
#include "h7/common/halloc.h"

typedef struct hstring hstring;

static inline char* hstrdup(const char* str){
    char* data = ALLOC(strlen(str) + 1);
    strcpy(data, str);
    return data;
}

//latter will opt.
hstring* hstring_append(hstring* owner, const char *str);
hstring* hstring_append2(hstring* owner, const char *str, uint32 len);
hstring* hstring_appendf(hstring* owner, const char *format, ...);
char* hstring_tostring(hstring* owner);

hstring* hstring_new();
void hstring_delete(hstring* owner);


#endif // H_STRING_H
