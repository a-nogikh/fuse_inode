#include "inode_data.h"
#include "fs.h"


int fs_get_disk_block_id(opened_file *handle, int seq_block){
    inode_t *inode = handle->inode;
    int now_blocks = inode->blocks_count;
    int i = seq_block < now_blocks ? seq_block : now_blocks;

    block_n return_block = 0;

    for (; i <= seq_block; i++){
        if (i < INODE_INNER_BLOCKS){
            if (inode->blocks[i] != INODE_EMPTY_BLOCK){
                return_block = inode->blocks[i];
                continue;
            }

            block_n new_block = create_disk_block(handle);
            if (new_block < 0){
                return -1;
            }

            inode->blocks[i] = new_block;
            inode->blocks_count++;
            handle->flushed = 0;
            return_block = inode->blocks[i];
        }
        else if (i >= INODE_INNER_BLOCKS && (i < INODE_INNER_BLOCKS + INODE_LINKS_PER_BLOCK)){
            if (inode->direct_pointers != INODE_EMPTY_BLOCK){
                load_block(handle, inode->direct_pointers, CACHED_DIRECT_POINTERS);
            }
            else if (create_block(handle, & inode->direct_pointers, CACHED_DIRECT_POINTERS)){
                handle->flushed = 0;
            }
            else{
                return -1;
            }

            block_n *level_1 = (block_n *)handle->cached_blocks[CACHED_DIRECT_POINTERS];
            int pos = (i-INODE_INNER_BLOCKS);
            if (level_1[pos] != INODE_EMPTY_BLOCK){
                return_block = level_1[pos];
                continue;
            }

            block_n new_block = create_disk_block(handle);
            if (new_block < 0){
                return -1;
            }

            level_1[pos] = new_block;
            inode->blocks_count++;
            handle->flushed = 0;
            handle->cached_block_flushed[CACHED_DIRECT_POINTERS] = 0;
            return_block = new_block;
        }
        else{
            int pos = i - INODE_INNER_BLOCKS - INODE_LINKS_PER_BLOCK;
            if (inode->indirect_pointers != INODE_EMPTY_BLOCK){
                 load_block(handle, inode->indirect_pointers, CACHED_INDIRECT_1);
            }
            else if (create_block( handle, &inode->indirect_pointers, CACHED_INDIRECT_1)){
                handle->flushed = 0;
            }
            else{
                return -1;
            }

            int pos_1 = pos / INODE_LINKS_PER_BLOCK, pos_2 = pos % INODE_LINKS_PER_BLOCK;
            block_n *level_2 = (block_n *)handle->cached_blocks[CACHED_INDIRECT_1];

            if(level_2[pos_1] != INODE_EMPTY_BLOCK){
                load_block(handle, level_2[pos_1], CACHED_INDIRECT_2);
            }
            else if (!create_block(handle, level_2 + pos_1, CACHED_INDIRECT_2)){
                return -1;
            }
            else{
                handle->cached_block_flushed[CACHED_INDIRECT_1] = 0;
            }

            block_n *blocks = (block_n *)handle->cached_blocks[CACHED_INDIRECT_2];
            if (blocks[pos_2] != INODE_EMPTY_BLOCK){
                return_block = blocks[pos_2];
                continue;
            }

            block_n new_block = create_disk_block(handle);
            if (new_block < 0){
                return -1;
            }

            blocks[pos_2] = new_block;
            handle->cached_block_flushed[CACHED_INDIRECT_2] = 0;
            inode->blocks_count++;
            handle->flushed = 0;
            return_block = new_block;
        }
    }

    inode_flush_data(handle);
    return return_block;
}

 block_n create_disk_block(opened_file *instance){
    int pos = bitmap_find(instance->disk_bitmap, -1);
    if (pos < 0){
        return -1;
    }

    bitmap_set(instance->disk_bitmap, pos, 1);
    device_clear_block(pos + instance->meta->disk_first_block);
    return pos;
}

static void fill_block(char *block, block_n val){
    int i = 0;
    for (; i < INODE_LINKS_PER_BLOCK; i++){
        *((block_n *)(block) + i) = val;
    }
}

void load_block(opened_file *opened, block_n dest_id, int type){
    if (opened->cached_block_ids[type] == dest_id){
        return;
    }
    else if(opened->cached_block_ids[type] != INODE_EMPTY_BLOCK
             && opened->cached_block_flushed[type] == 0){
        device_write_block(opened->meta->disk_first_block + opened->cached_block_ids[type],
                           opened->cached_blocks[type]);
    }

    opened->cached_block_ids[type] = dest_id;
    device_read_block(dest_id + opened->meta->disk_first_block, opened->cached_blocks[type]);
    opened->cached_block_flushed[type] = 1;
}

int create_block(opened_file *opened, block_n *dest_id, int type){
    if(*dest_id != INODE_EMPTY_BLOCK && opened->cached_block_flushed[type] == 0){
        device_write_block(opened->meta->disk_first_block + opened->cached_block_ids[type],
                   opened->cached_blocks[type]);
    }

    fill_block(opened->cached_blocks[type], -1);
    int disk_n = bitmap_find(opened->disk_bitmap, -1);
    if (disk_n < 0){
        return -1;
    }
    bitmap_set(opened->disk_bitmap, disk_n, 1);
    device_write_block(disk_n + opened->meta->disk_first_block, opened->cached_blocks[type]);
    opened->cached_block_ids[type] = disk_n;
    opened->cached_block_flushed[type] = 1;
    *dest_id = disk_n;
}

void inode_flush_data(opened_file *opened)
{
    int i = 0;
    for (; i < CACHED_COUNT; i++){
        if (opened->cached_block_flushed[i] == 1){
            continue;
        }
        opened->cached_block_flushed[i] = 1;
        device_write_block(
           opened->meta->disk_first_block + opened->cached_block_ids[i],
           opened->cached_blocks[i]);
    }

    if(opened->flushed == 0){
        opened->flushed = 1;
        inode_save(opened->inode);
    }
}
