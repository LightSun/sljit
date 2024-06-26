
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "h_string.h"
#include "h7/common/c_common.h"

typedef struct StringFragment StringFragment;

struct StringFragment {
    struct StringFragment *next;
    uint32 length;
    char *str;
};
struct hstring{
    StringFragment* head;
    StringFragment* cur;
    int total_len;
    char* total_buf;
};

hstring* hstring_new(){
    hstring* owner = ALLOC(sizeof (hstring));
    owner->head = owner->cur = NULL;
    owner->total_len = 0;
    owner->total_buf = 0;
    return owner;
}

hstring* hstring_append(hstring* owner, const char *str){
    return hstring_append2(owner, str, (uint32)strlen(str));
}

hstring* hstring_append2(hstring* owner, const char *str, uint32 len){
    StringFragment *frag = NULL;
    if (NULL == str || '\0' == *str)
        return owner;
    frag = ALLOC(sizeof (StringFragment));

    frag->next = NULL;
    frag->length = len;
    frag->str = ALLOC(len + 1);
    //strcpy(frag->str, str);
    memcpy(frag->str, (const void *)str, len);
    frag->str[len] = '\0';

    if(owner->head == NULL){
        owner->head = owner->cur = frag;
    }else{
        owner->cur->next = frag;
        owner->cur = frag;
    }
    owner->total_len += len;
    return owner;
}

#define HSTRING_BUF_LEN 1024
hstring* hstring_appendf(hstring* owner, const char *format,...){
    if (format == NULL) {
        printf("wrong format\n");
        return owner;
    }
    int rc = 0;
    char buf[HSTRING_BUF_LEN] = {0}; //'\0'
    va_list args;

    va_start(args, format);
    rc = vsnprintf(&buf[0], HSTRING_BUF_LEN, format, args);
    va_end(args);

    if (rc <= 0)
        return owner;

    return hstring_append2(owner, buf, (uint32)rc);
}
char* hstring_tostring(hstring* owner){
    if(owner->total_buf == NULL){
        owner->total_buf = ALLOC(owner->total_len + 1);
    }else if((int)strlen(owner->total_buf) < owner->total_len){
        FREE(owner->total_buf);
        owner->total_buf = ALLOC(owner->total_len + 1);
    }
    char *c = NULL;
    StringFragment *frag = NULL;

    c = owner->total_buf;
    for (frag = owner->head; frag; frag = frag->next) {
        //strncpy(c, frag->str, frag->length);
        memcpy(c, frag->str, frag->length);
        c += frag->length;
    }
    *c = '\0';
    //owner->total_buf[owner->total_len] = '\0';
    return owner->total_buf;
}

void hstring_log_and_delete(hstring* owner){
    char* s = hstring_tostring(owner);
    LOGI("%s\n", s);
    hstring_delete(owner);
}

void hstring_delete(hstring* owner){
    if(owner->total_buf){
        FREE(owner->total_buf);
    }
    StringFragment *frag = NULL;
    StringFragment *old = NULL;
    for (frag = owner->head; frag; ) {
        old = frag;
        frag = frag->next;
        FREE(old->str);
        FREE(old);
    }
    FREE(owner);
}
