
#include "array_map_n.h"
#include <memory.h>
#include "hash.h"
#include "binary_search.h"
#include "mem.h"

#define DEFAULT_HASH_COUNT 8

#define prepare_key_size(ptr, delta) \
    array_map_n_prepare_keySize(ptr, delta + ptr->alloc_key_size)

#define prepare_value_size(ptr, delta)\
    array_map_n_prepare_valueSize(ptr, delta + ptr->alloc_val_size)

#define prepare_entry_count( ptr,  delta)\
    array_map_n_prepare_entryCount(ptr, ptr->hash_count + delta)

array_map_n_p array_map_n_new(uint32 init_key_bytes, uint32 init_val_bytes){
    array_map_n_p ptr = (array_map_n_p)H7_NEW_TYPE(struct array_map_n);

    ptr->keys = init_key_bytes > 0 ? H7_NEW(init_key_bytes) : NULL;
    ptr->values = init_val_bytes > 0 ? H7_NEW(init_val_bytes) : NULL;
    ptr->key_hashes = H7_NEW_COUNT_TYPE(uint32, DEFAULT_HASH_COUNT);
    ptr->key_lens = H7_NEW_COUNT_TYPE(uint16, DEFAULT_HASH_COUNT);
    ptr->val_lens = H7_NEW_COUNT_TYPE(uint16, DEFAULT_HASH_COUNT);

    ptr->alloc_key_size = init_key_bytes;
    ptr->alloc_val_size = init_val_bytes;
    ptr->hash_count = DEFAULT_HASH_COUNT;
    ptr->len_entry = 0;
    return ptr;
}

void array_map_n_put(array_map_n_p ptr, const void* key, uint16 key_size,
                     const void* val, uint16 val_size,
                     struct _MemoryBlock* out){
    if(out){
        out->data = NULL;
    }
    uint32 hash = fasthash32(key, key_size, 0);
    if(ptr->len_entry == 0){
        prepare_key_size(ptr, key_size);
        prepare_value_size(ptr, val_size);
        prepare_entry_count(ptr, 1);
        memcpy(ptr->keys, key, key_size);
        memcpy(ptr->values, val, val_size);
        memcpy(ptr->key_hashes, &hash, sizeof (uint32));
        ptr->key_lens[0] = key_size;
        ptr->val_lens[0] = val_size;
        ptr->len_entry = 1;
        return;
    }
    int pos = binarySearch_uint32(ptr->key_hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        uint16 old_val_len = ptr->val_lens[pos];
        prepare_value_size(ptr, val_size - old_val_len);
        //compute key need move size
        uint32 offsetValSize = 0;
        for(int i = 0 ; i < pos ; ++ i){
            offsetValSize += ptr->val_lens[i];
        }
        uint32 postOffsetValSize = 0;
        for(uint32 i = pos + 1 ; i < ptr->len_entry ; ++ i){
            postOffsetValSize += ptr->val_lens[i];
        }
        //copy old value if need
        if(out){
            out->size = old_val_len;
            out->data = H7_NEW(old_val_len);
            memcpy(out->data, (char*)ptr->values + offsetValSize, old_val_len);
        }
        //override old value
        void* tmp_vals = H7_NEW(postOffsetValSize);
        memcpy(tmp_vals, (char*)ptr->values + offsetValSize + old_val_len, postOffsetValSize);
        memcpy((char*)ptr->values + offsetValSize , val, val_size);
        memcpy((char*)ptr->values + offsetValSize + val_size, tmp_vals, postOffsetValSize);
        H7_DELETE(tmp_vals);
        ptr->val_lens[pos] = val_size;
    }else{
        prepare_key_size(ptr, key_size);
        prepare_value_size(ptr, val_size);
        prepare_entry_count(ptr, 1);
        int pos1 = - (pos +1);
        //compute key need move size
        uint32 offsetKeySize = 0;
        uint32 offsetValSize = 0;
        for(int i = 0 ; i < pos1 ; ++ i){
            offsetKeySize += ptr->key_lens[i];
            offsetValSize += ptr->val_lens[i];
        }

        uint32 postOffsetKeySize = 0;
        uint32 postOffsetValSize = 0;
        for(uint32 i = pos1 ; i < ptr->len_entry ; ++ i){
            postOffsetKeySize += ptr->key_lens[i];
            postOffsetValSize += ptr->val_lens[i];
        }
        //move keys and values which need change position
        void* tmp_keys = H7_NEW(postOffsetKeySize);
        void* tmp_vals = H7_NEW(postOffsetValSize);
        memcpy(tmp_keys, (char*)ptr->keys + offsetKeySize, postOffsetKeySize);
        memcpy(tmp_vals, (char*)ptr->values + offsetValSize, postOffsetValSize);
        // insert new key and value
        memcpy((char*)ptr->keys + offsetKeySize, key, key_size);
        memcpy((char*)ptr->values + offsetValSize, val, val_size);
        memcpy((char*)ptr->keys + offsetKeySize + key_size, tmp_keys, postOffsetKeySize);
        memcpy((char*)ptr->values + offsetValSize + val_size, tmp_vals, postOffsetValSize);
        //insert key and val info
        arrays_insert(ptr->key_lens, ptr->len_entry, sizeof (uint16), &key_size, pos1);
        arrays_insert(ptr->val_lens, ptr->len_entry, sizeof (uint16), &val_size, pos1);
        arrays_insert(ptr->key_hashes, ptr->len_entry, sizeof (uint32), &hash, pos1);
        //free
        H7_DELETE(tmp_keys);
        H7_DELETE(tmp_vals);
        ptr->len_entry += 1;
    }
}

void array_map_n_get(array_map_n_p ptr, const void* key, uint16 key_size,
                     struct _MemoryBlock* out){
    ASSERT(out != NULL);
    out->data = NULL;
    uint32 hash = fasthash32(key, (uint32)key_size, 0);
    int pos = binarySearch_uint32(ptr->key_hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        uint16 old_val_len = ptr->val_lens[pos];
        out->size = old_val_len;
        out->data = H7_NEW(old_val_len);
        //compute key need move size
        uint32 offsetValSize = 0;
        for(int i = 0 ; i < pos ; ++ i){
            offsetValSize += ptr->val_lens[i];
        }
        memcpy(out->data, (char*)ptr->values + offsetValSize, old_val_len);
    }
}


void array_map_n_rawget(array_map_n_p ptr, const void* key, uint16 key_size,
                        struct _MemoryBlock* out){
    ASSERT(out != NULL);
    out->data = NULL;
    uint32 hash = fasthash32(key, (uint32)key_size, 0);
    int pos = binarySearch_uint32(ptr->key_hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        uint16 old_val_len = ptr->val_lens[pos];
        out->size = old_val_len;
        //compute key need move size
        uint32 offsetValSize = 0;
        for(int i = 0 ; i < pos ; ++ i){
            offsetValSize += ptr->val_lens[i];
        }
        out->data = (char*)ptr->values + offsetValSize;
    }
}

int array_map_n_remove(array_map_n_p ptr, const void* key, uint16 key_size,
                        struct _MemoryBlock* out){
    if(out){
        out->data = NULL;
    }
    uint32 hash = fasthash32(key, key_size, 0);
    int pos = binarySearch_uint32(ptr->key_hashes, 0, ptr->len_entry, hash);
    if(pos >= 0){
        uint16 old_val_len = ptr->val_lens[pos];

        //compute key need move size
        uint32 offsetValSize = 0;
        uint32 offsetKeySize = 0;
        for(int i = 0 ; i < pos ; ++ i){
            offsetValSize += ptr->val_lens[i];
            offsetKeySize += ptr->key_lens[i];
        }
        if(out){
            out->size = old_val_len;
            out->data = H7_NEW(old_val_len);
            memcpy(out->data, (char*)ptr->values + offsetValSize, old_val_len);
        }
        //remove
        mem_remove(ptr->keys, ptr->alloc_key_size, offsetKeySize, ptr->key_lens[pos]);
        mem_remove(ptr->values, ptr->alloc_val_size, offsetValSize, old_val_len);
        arrays_remove(ptr->key_lens, ptr->hash_count, sizeof (uint16), pos);
        arrays_remove(ptr->val_lens, ptr->hash_count, sizeof (uint16), pos);
        arrays_remove(ptr->key_hashes, ptr->hash_count, sizeof (uint32), pos);

        ptr->len_entry --;
        return 1;
    }
    return 0;
}

array_map_n_p array_map_n_copy(array_map_n_p ptr1){
    array_map_n_p ptr = (array_map_n_p)H7_NEW(sizeof (struct array_map_n));

    ptr->keys = ptr1->alloc_key_size > 0 ? H7_NEW(ptr1->alloc_key_size) : NULL;
    ptr->values = ptr1->alloc_val_size > 0 ? H7_NEW(ptr1->alloc_val_size) : NULL;
    ptr->key_hashes = H7_NEW_COUNT_TYPE(uint32, ptr1->hash_count);
    ptr->key_lens = H7_NEW_COUNT_TYPE(uint16, ptr1->hash_count);
    ptr->val_lens = H7_NEW_COUNT_TYPE(uint16, ptr1->hash_count);

    ptr->alloc_key_size = ptr1->alloc_key_size;
    ptr->alloc_val_size = ptr1->alloc_val_size;
    ptr->hash_count = ptr1->hash_count;
    ptr->len_entry = ptr1->len_entry;
    //copy data
    if(ptr->keys){
        memcpy(ptr->keys, ptr1->keys, ptr1->alloc_key_size);
    }
    if(ptr->keys){
        memcpy(ptr->values, ptr1->values, ptr1->alloc_val_size);
    }
    memcpy(ptr->key_hashes, ptr1->key_hashes, ptr1->hash_count * sizeof (uint32));
    memcpy(ptr->key_lens, ptr1->key_lens, ptr1->hash_count * sizeof (uint16));
    memcpy(ptr->val_lens, ptr1->val_lens, ptr1->hash_count * sizeof (uint16));
    return ptr;
}

void array_map_n_delete(array_map_n_p ptr){
    if(ptr->keys){
        H7_DELETE(ptr->keys);
    }
    if(ptr->values){
        H7_DELETE(ptr->values);
    }
    H7_DELETE(ptr->key_hashes);
    H7_DELETE(ptr->key_lens);
    H7_DELETE(ptr->val_lens);
    H7_DELETE(ptr);
}
