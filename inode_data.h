#ifndef INODE_DATA_H_INCLUDED
#define INODE_DATA_H_INCLUDED

#include "fs.h"
#include "inode.h"

#define CACHED_LEVEL_1 0
#define CACHED_LEVEL_2_1 1
#define CACHED_LEVEL_2_2 2

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


#endif // INODE_DATA_H_INCLUDED
