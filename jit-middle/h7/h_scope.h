#ifndef H_SCOPE_H
#define H_SCOPE_H

typedef struct array_map array_map;
typedef struct hscope hscope;
typedef struct IClass IClass;

typedef struct hscope{
    hscope* parent;
    array_map* obj_pool; //kv = name, obj
    array_map* obj_base;
    array_map* type_base;//kv = string, int/char...etc.
    array_map* type_pool;//kv = string, iclass
}*hscope_p;

hscope_p hscope_new(hscope* parent);

hscope_p hscope_get_current();
void hscope_set_current(hscope_p p);

void hscope_put_obj(hscope_p cur, const char* type, void* obj);
void* hscope_get_obj(hscope_p cur, const char* type);

void hscope_put_type(hscope_p cur, const char* type, IClass* cls);
IClass* hscope_get_type(hscope_p cur, const char* type);

#endif // H_SCOPE_H
