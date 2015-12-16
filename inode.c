#include "inode.h"
#include "device.h"
#include "utils.h"
#include <stdlib.h>

static int starting_block, ending_block;
static int inode_count;
static int per_block = BLOCK_SIZE / sizeof(inode);

void inode_init(int s_block, int e_block){
    starting_block = s_block;
    ending_block = e_block;
    inode_count = (e_block - s_block + 1) * per_block;
}

int inode_get_size(int nodes){
    return DIV_ROUND_UP(nodes, per_block);
}

inode *inode_find(int id){
    int block_id = id / per_block + starting_block;
    if (block_id > ending_block){
        return NULL;
    }

    inode *result = (inode *)malloc(sizeof(inode));
    device_read_block_ofs(block_id, (char *)result, (id % per_block) * sizeof(inode), sizeof(inode));
    return result;
}

void inode_save(inode *inode){
    int block_id = inode->id / per_block + starting_block;
    if (block_id <= ending_block){
        device_write_block_ofs(block_id, (char *)inode, id % per_block, sizeof(inode));
    }
}

inode *inode_make(bitmap_instance *bitmap){
    int free_inode = bitmap_find(bitmap, 0);
    if (free_inode < 0){
        return NULL;
    }

    bitmap_set(bitmap, free_inode, 1);
    inode *inode = (inode *)malloc(sizeof(inode));
    inode->id = free_inode;
    int i = 0;
    for (; i < INODE_INNER_BLOCKS; i++){
        inode->blocks[i] = INODE_EMPTY_BLOCK;
    }

    inode->direct_pointers = INODE_EMPTY_BLOCK;
    inode->indirect_pointers = INODE_EMPTY_BLOCK;

    return inode;
}

void inode_unlink(bitmap_instace *bitmap, inode *node)
{
    bitmap_set(bitmap, node->id, 0);

}


void inode_free(inode *inode){
    free(inode);
}
