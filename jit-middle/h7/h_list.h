#ifndef H_LIST_H
#define H_LIST_H

#include "h7/common/dtypes.h"

typedef struct array_list{
   void** data;
   int element_count;
   int max_count;
   float factor; //the load factor
}array_list, *array_list_p;

#define array_list_new_simple() array_list_new(8, 0.75f)
#define array_list_new2(init_count) array_list_new(init_count, 0.75f)
#define array_list_new_max(count) array_list_new(count * 4 / 3 + 1, 0.75f)
array_list* array_list_new(int init_count, float factor);

/**
 * @brief array_list_delete: delete/release the list with all memory
 * @param list: the list
 * @param Func: the Func to release element
 * @param ud the userdata
 */
void array_list_delete(array_list* list,
                       dt_func_delete func, void* ud);

void array_list_delete2(array_list* list, void (*Func)(void* ele));

void array_list_ensure_capacity(array_list* list, int cap);

void array_list_ensure_size(array_list* list, int size);

int array_list_size(array_list* list);

void array_list_add(array_list* list, void* ele);

void array_list_addI(array_list* list, int index, void* ele);

//careful, you may need to free old before this
static inline void array_list_set(array_list* list,
                                  int index, void* ele){
    ASSERT(index < list->element_count);
    list->data[index] = ele;
}
/**
 * @brief array_list_remove: remove element from array
 * @param array_list: the list
 * @param ele: the element
 * @param Func: the function to judge the element is the same or not.
 * @return the old data. or NULL if not found
 */
void* array_list_remove(array_list* list, void* ele, void* ud, dt_func_eq func);

void* array_list_remove_at(array_list* list, int index);

//int dtype_obj_equals(void* ud, void* ele1, void* ele2);
int array_list_index_of(array_list* list, void* val, void* ud,
                        dt_func_eq func);

/**
 * @brief array_list_get: get element by target index
 * @param list the list
 * @param index the index
 * @return the element if found, or NULL
 */
void* array_list_get(array_list* list, int index);
/**
 * @brief array_list_remove_all: remove element from list by target function
 * @param list: the list
 * @param Func: the function ptr. return 0 if found
 * @param out_list: the remove element list. can be null
 * @return remove count
 */
int array_list_remove_all(array_list* list, void* val, void* ud,
                          dt_func_eq func, array_list* out_list);

/**
 * @brief array_list_find_all: found the all match elements
 * @param list the list
 * @param Func the function ptr. return 0 if found
 * @param out_list the out list.
 */
void array_list_find_all(array_list* list,void* val, void* ud,
                         dt_func_eq func, array_list* out_list);


array_list* array_list_copy(array_list* list, void* ud,
            dt_func_cpy func);
uint32 array_list_hash(array_list* list,
                            void* ud, uint32 seed, dt_func_hash func);

#endif // H_LIST_H
