#ifndef INODE_DATA_H_INCLUDED
#define INODE_DATA_H_INCLUDED

#include "fs.h"
#include "inode.h"

#define CACHED_DIRECT_POINTERS 0
#define CACHED_INDIRECT_1 1
#define CACHED_INDIRECT_2 2

struct opened_file {
    inode *inode;
    meta *meta;
    bitmap_instance *disk_bitmap;
    bitmap_instance *inode_bitmap;

    int flushed;

    char cached_blocks[3][BLOCK_SIZE];
    block_n cached_block_ids[3];
    int cached_block_flushed[3];
};
typedef struct opened_file opened_file;

int fs_get_block_id(opened_file *handle, bitmap *disk_bitmap, int offset);

#endif // INODE_DATA_H_INCLUDED
