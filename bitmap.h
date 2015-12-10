#ifndef BITMAP_H_INCLUDED
#define BITMAP_H_INCLUDED

#typedef char type_8bit

struct bitmap_instance{
    type_8bit *bitmap_data;
    int unflashed_block_id;
    int from_block, to_block;
    int bits_count;
};


#endif // BITMAP_H_INCLUDED
