#ifndef INODE_DATA_H_INCLUDED
#define INODE_DATA_H_INCLUDED

#include "fs.h"
#include "inode.h"

#define CACHED_DIRECT_POINTERS 0
#define CACHED_INDIRECT_1 1
#define CACHED_INDIRECT_2 2
#define CACHED_COUNT 3

struct opened_file {
    inode *inode;
    meta *meta;
    bitmap_instance *disk_bitmap;
    bitmap_instance *inode_bitmap;

    int flushed;

    char cached_blocks[CACHED_COUNT][BLOCK_SIZE];
    block_n cached_block_ids[CACHED_COUNT];
    int cached_block_flushed[CACHED_COUNT];
};
typedef struct opened_file opened_file;

int fs_get_disk_block_id(opened_file *handle, int seq_block);
int inode_flush_data(opened_file *opened)

#endif // INODE_DATA_H_INCLUDED
