#ifndef BITMAP_H_INCLUDED
#define BITMAP_H_INCLUDED

typedef char type_8bit;

struct bitmap_instance{
    type_8bit *bitmap_data;
    int unflashed_block_id;
    int from_block, to_block;
    int bits_count;
    int curr_bit;
};

typedef struct bitmap_instance bitmap_instance;

bitmap_instance * bitmap_init(int from_block, int to_block);
int bitmap_get_blocks_count(int bits_needed);
int bitmap_bits_from_blocks(int blocks);
int bitmap_find(bitmap_instance *instance, int from_bit);
int bitmap_get(bitmap_instance *instance, int bit);
void bitmap_set(bitmap_instance *instance, int bit, int v);
void bitmap_clear(bitmap_instance *instance);
void bitmap_flush(bitmap_instance *instance);
void bitmap_free(bitmap_instance *instance);

#endif // BITMAP_H_INCLUDED
