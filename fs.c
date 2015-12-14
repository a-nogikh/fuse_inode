#include "fs.h"

int fs_get_block_id(opened_file *handle, bitmap *disk_bitmap, int offset){
    inode *inode = handle->inode;
    int block = offset / BLOCK_SIZE, now_blocks = inode->blocks_count;
    int i = block < now_blocks ? block : now_blocks;


    int last_fetched_block = 0;
    for (; i <= block; i++){
        if (i < INODE_INNER_BLOCKS && inode->blocks[i] == INODE_EMPTY_BLOCK){
            int new_block = bitmap_find(disk_bitmap, last_fetched_block);
            if (new_block == -1){
                return -1;
            }
        }

    }
}
