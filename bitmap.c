#include "bitmap.h"
#include "device.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>

bitmap_instance * bitmap_init(int from_block, int to_block){
    int bytes = (to_block - from_block + 1) * BLOCK_SIZE;
    bitmap_instance *instance = (bitmap_instance *)malloc(sizeof(bitmap_instance));
    instance->bitmap_data = (type_8bit *)malloc(sizeof(type_8bit) * bytes)
    instance->bits_count = bytes * 8;
    instance->from_block = from_block;
    instance->to_block = to_block;
    instance->unflashed_block_id = -1;

    int i;
    for (i = from_block; i <= to_block; i++){
        device_read_block(i, instance->bitmap_data + BLOCK_SIZE*(i-from_block));
    }

    return instance;
}

int bitmap_get_blocks_count(int bits_needed){
    return DIV_ROUND_UP(DIV_ROUND_UP(bits_needed, 8), BLOCK_SIZE);
}

void bitmap_set(bitmap_instance *instance, int bit, int v){
    //
}

void bitmap_clear(bitmap_instance *instance){
    memset(instance->bitmap_data, 0, sizeof(type_8bit));
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
}
