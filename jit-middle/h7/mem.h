#ifndef H7_MEM_H
#define H7_MEM_H

#include "h7/h7_common.h"
#include <string.h>

#define DEF_ARRAY_INSERT_ONE(_data, ele_c, unit_size, ptr, index)\
if((uint32)index < (uint32)ele_c){\
    uint32 _ele_c = (uint32)ele_c;\
    uint32 _unit_size = (uint32)unit_size;\
    uint32 _index = (uint32)index;\
    uint32 move_c = _ele_c - _index;\
    char* data = (char*)_data + _index * _unit_size;\
    memmove(data + move_c * _unit_size, data, move_c * _unit_size);\
}\
if(ptr){\
    char* data = (char*)_data + (uint32)index * (uint32)unit_size;\
    memcpy(data, ptr, (uint32)unit_size);\
}

#define DEF_ARRAY_REMOVE_ONE(_data, ele_c, unit_size, index)\
{\
    uint32 _move_c = (uint32)(ele_c - (uint32)index - 1);\
    if(_move_c > 0){\
        uint32 _unit_size = (uint32)unit_size;\
        char* data = (char*)_data + (uint32)index * _unit_size;\
        memmove(data, data + _unit_size, (uint32)_move_c * _unit_size);\
    }\
}

void arrays_insert(void* arr, unsigned int already_ele_count,
                   unsigned int ele_unit_size,
                   const void* data, unsigned int target_pos);

void arrays_remove(void* arr, unsigned int already_ele_count,
                   unsigned int ele_unit_size,
                   unsigned int target_pos);

/**
 * @brief mem_remove remove the memory
 * @param mem  the memory addr
 * @param total_size the total size of the mem
 * @param offset the offset of memory block which will be remove
 * @param size the size of memory block to remove
 */
void mem_remove(void* mem, unsigned int total_size,
                unsigned int offset, unsigned int size);


/**
 * @brief mem_insert insert data to memory
 * @param mem the memory addr
 * @param total_size the total size of the mem except the data size
 * @param offset the offset of memory block which will be insert
 * @param data the new data which will be insert
 * @param size the data size
 */
void mem_insert(void* mem, unsigned int total_size,
                unsigned int offset, const void* data, unsigned int size);


#endif
