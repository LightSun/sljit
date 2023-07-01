#include <string.h>
#include "h_list.h"
#include "h7/common/halloc.h"
#include "h7/common/dtypes.h"

#define MIN_COUNT 8

static inline void __grow(array_list* list, int minCapacity){
   int oldCapacity = list->max_count;
   int newCapacity = oldCapacity + (oldCapacity >> 1);
   if (newCapacity - minCapacity < 0)
       newCapacity = minCapacity;
   //
    list->max_count = newCapacity;
    void* new_data = REALLOC(list->data, 0, sizeof (void*) * (uint32)newCapacity);
    if(new_data == NULL){
        void** old = list->data;
        list->data = ALLOC(sizeof (void*) * (uint32)newCapacity);
        memcpy(list->data, old, sizeof (void*) * (uint32)oldCapacity);
        FREE(old);
    }else{
        list->data = new_data;
    }
}
void array_list_ensure_capacity(array_list* list, int cap){
    if(list->max_count < cap){
        __grow(list, cap);
    }
}
void array_list_ensure_size(array_list* list, int size){
    array_list_ensure_capacity(list, (int)((float)size / list->factor) + 1);
    list->element_count = size;
}

array_list* array_list_new(int init_count, float factor){
    if(factor > 1 || init_count <= 0){
        return NULL;
    }
    array_list* l = ALLOC(sizeof (array_list));
    l->data = ALLOC((uint32)init_count * sizeof (void*));
    l->max_count = init_count;
    l->element_count = 0;
    l->factor = factor;
    return l;
}
void array_list_delete(array_list* list, dt_func_delete func, void* ud){
    if(func){
        for(int i = 0; i < list->element_count; i ++){
            func(ud, list->data[i]);
        }
    }
    FREE(list->data);
    FREE(list);
}

void array_list_delete2(array_list* list, void (*Func)(void* ele)){
    if(Func){
        for(int i = 0; i < list->element_count; i ++){
            Func(list->data[i]);
        }
    }
    FREE(list->data);
    FREE(list);
}

int array_list_size(array_list* list){
    return list->element_count;
}
void array_list_add(array_list* list, void* ele){
    list->data[list->element_count++] = ele;
    if(list->element_count >= (int)(list->factor * (float)list->max_count)){
        __grow(list, 0);
    }
}
void array_list_addI(array_list* list, int index, void* ele){
    if(index > list->element_count){
        index = list->element_count;
    }
    if(list->element_count + 1 >= (int)(list->factor * (float)list->max_count)){
        __grow(list, 0);
    }
    if(index < list->element_count){
        //move
        uint32 offset = (uint32)index * sizeof (void*);
        uint32 total_c = (uint32)(list->element_count - index) * sizeof (void*);
        char* data = (char*)list->data + offset;
        memmove(data + sizeof (void*), data, total_c);
    }
    list->data[index] = ele;
    list->element_count ++;
}
void* array_list_get(array_list* list, int idx){
    if(idx >= list->element_count){
        return NULL;
    }
    return list->data[idx];
}
void* array_list_remove_at(array_list* list, int idx){
    if(idx < 0 || idx >= list->element_count){
        return NULL;
    }
    void* old_data = list->data[idx];
    // 3,  0 -> 2  :: 3, 1 -> 1 ::3, 2 -> 0
    int move_c = list->element_count - idx - 1;
    if(move_c > 0){
        char* data = (char*)list->data + (uint32)idx * sizeof (void*);
        memmove(data, data + sizeof (void*),
                 (uint32)move_c * sizeof (void*));
    }
    list->element_count -- ;
    return old_data;
}

int array_list_index_of(array_list* list, void* val, void* ud,
                        dt_func_eq func_eq){
    for(int i = 0; i < list->element_count; i ++){
        if(func_eq(ud, list->data[i], val)){
            return i;
        }
    }
    return -1;
}

void* array_list_remove(array_list* list, void* ele, void* ud, dt_func_eq func){
    int idx = -1;
    void* old_data = NULL;
    for(int i = list->element_count - 1; i >=0 ; i --){
        if( (func && func(ud, list->data[i], ele))){
            idx = i;
            old_data = list->data[i];
            break;
        }
    }
    //remove element
    if( idx >= 0){
        // 3,  0 -> 2  :: 3, 1 -> 1
        uint32 size = sizeof (void*) * (uint32)(list->element_count - 1 - idx);
        char* data = (char*)list->data + (uint32)idx * sizeof (void*);
        if(size > 0){
            memmove(data, data + sizeof (void*), size);
        }
        list->element_count -- ;
    }
    return old_data;
}

void array_list_find_all(array_list* list,void* val, void* ud,
                         dt_func_eq func, array_list* out_list){
    for(int i = 0; i < list->element_count; i ++){
        if(func(ud, val, list->data[i])){
            array_list_add(out_list, list->data[i]);
        }
    }
}
int array_list_remove_all(array_list* list, void* val, void* ud,
                          dt_func_eq func, array_list* out_list){
    uint32 old_count = (uint32)list->element_count;
    int remove_count = 0;
    //remove mark
    signed char idxes[old_count];
    memset(idxes, 0, (uint32)old_count * sizeof (int));
    for(uint32 i = 0 ; i < old_count ; i ++){
        if(func(ud, val, list->data[i])){
            if(out_list){
                array_list_add(out_list, list->data[i]);
            }
            idxes[i] = 1;
            remove_count ++;
        }
    }
    if(remove_count > 0){
        list->element_count = (int)old_count -  remove_count;
        void** startPtr;
        for(uint32 i = 0 ; i < old_count ; i ++){
            //move
            if(idxes[i] == 1){
                int c = 1;
                for(uint32 j = i + 1; j < old_count ; j ++){
                    if(idxes[j] == 1){
                        c ++;
                    }else{
                        break;
                    }
                }
                startPtr = list->data + i * sizeof (void*);
                memmove(startPtr, startPtr + (uint32)c * sizeof (void*), (uint32)c * sizeof (void*));
            }
        }
    }
    return remove_count;
}
array_list* array_list_copy(array_list* list,
                       void* ud, dt_func_cpy func){
    array_list* ret = array_list_new(list->max_count, list->factor);
    for(int i = 0; i < list->element_count; i ++){
        void* ele = func(ud, list->data[i]);
        ret->data[i] = ele;
    }
    return ret;
}

uint32 array_list_hash(array_list* list,
            void* ud, uint32 seed, dt_func_hash func){
    for(int i = 0; i < list->element_count; i ++){
        seed = func(ud, list->data[i], seed);
    }
    return seed;
}

