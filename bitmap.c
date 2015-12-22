#include "bitmap.h"
#include "device.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

bitmap_instance * bitmap_init(int from_block, int to_block){
    int bytes = (to_block - from_block + 1) * BLOCK_SIZE;
    bitmap_instance *instance = (bitmap_instance *)malloc(sizeof(bitmap_instance));
    instance->bitmap_data = (type_8bit *)malloc(sizeof(type_8bit) * bytes);
    instance->bits_count = bytes * 8;
    instance->from_block = from_block;
    instance->to_block = to_block;
    instance->curr_bit = 0;
    instance->unflashed_block_id = -1;

    int i;
    for (i = from_block; i <= to_block; i++){
        device_read_block(i, instance->bitmap_data + BLOCK_SIZE*(i-from_block));
    }

    return instance;
}

int bitmap_get_blocks_count(int bits_needed){
    int val =  DIV_ROUND_UP(DIV_ROUND_UP(bits_needed, 8), BLOCK_SIZE);
    if (bits_needed > 0 && val == 0){
        return 1;
    }
    return val;
}

int bitmap_bits_from_blocks(int blocks){
    return blocks * BLOCK_SIZE * 8;
}

int bitmap_find(bitmap_instance *instance, int from_bit){
    if (from_bit == -1){
        from_bit = instance->curr_bit;
    }

    int offset = 0, count = instance->bits_count/8, from = from_bit / 8;
    for(; offset < count; offset++){
        type_8bit val = *(instance->bitmap_data + (offset + from) % count);
        if (~val == 0){ continue; }
        instance->curr_bit = ((offset + from) % count)*8;

        int j = (offset == 0) ? (from_bit % 8) : 0;
        for(; j < 8; j++){
            if (!(val & (1 << j))){
                return ((from + offset)%count) * 8 + j;
            }
        }
    }
    return -1;
}

int bitmap_get(bitmap_instance *instance, int bit){
    if (bit > instance->bits_count){
        return -1;
    }

    int res = *((type_8bit *)instance->bitmap_data + bit/8) & (1 << (bit%8));
    if (res > 0){
        return 1;
    }
    return 0;
}

void bitmap_set(bitmap_instance *instance, int bit, int v){
    if (bit > instance->bits_count){
        return;
    }

    int byte = bit/8, block = byte / BLOCK_SIZE;
    type_8bit *val = (type_8bit *)(instance->bitmap_data + byte);

    if (v == 0){
        *val &= ~(1 << (bit%8));
    }
    else{
        *val |= 1 << (bit%8);
    }

    if (instance->unflashed_block_id != block){
        bitmap_flush(instance);
    }

    instance->unflashed_block_id = block;
}

void bitmap_clear(bitmap_instance *instance){
    memset(instance->bitmap_data, 0, sizeof(type_8bit));
    int i = instance->from_block;
    for (; i <= instance->to_block; i++){
        device_write_block(i, instance->bitmap_data + i * BLOCK_SIZE);
    }
}

void bitmap_flush(bitmap_instance *instance){
    if (instance->unflashed_block_id < 0){
        return;
    }

    device_write_block(
        instance->from_block + instance->unflashed_block_id,
        instance->bitmap_data + BLOCK_SIZE * instance->unflashed_block_id
    );
    instance->unflashed_block_id = -1;
}

void bitmap_free(bitmap_instance *instance){
    free(instance->bitmap_data);
    free(instance);
}
