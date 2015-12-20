#include "inode.h"
#include "device.h"
#include "utils.h"
#include <stdlib.h>
#include "inode_data.h"

static int starting_block, ending_block;
static int inode_count;
static int per_block = BLOCK_SIZE / sizeof(inode_t);

void inode_init(int s_block, int e_block){
    starting_block = s_block;
    ending_block = e_block;
    inode_count = (e_block - s_block + 1) * per_block;
}

int inode_get_size(int nodes){
    return DIV_ROUND_UP(nodes, per_block);
}

inode_t *inode_find(int id){
    int block_id = id / per_block + starting_block;
    if (block_id > ending_block){
        return NULL;
    }

    inode_t *result = (inode_t *)malloc(sizeof(inode_t));
    device_read_block_ofs(block_id, (char *)result, (id % per_block) * sizeof(inode_t), sizeof(inode_t));
    return result;
}

void inode_save(inode_t *inode){
    int block_id = inode->id / per_block + starting_block;
    if (block_id <= ending_block){
        device_write_block_ofs(block_id, (char *)inode,
           (inode->id % per_block) * sizeof(inode), sizeof(inode));
    }
}

inode_t *inode_make(bitmap_instance *bitmap){
    int free_inode = bitmap_find(bitmap, 0);
    if (free_inode < 0){
        return NULL;
    }

    bitmap_set(bitmap, free_inode, 1);
    inode_t *node = (inode_t *)malloc(sizeof(inode_t));
    node->id = free_inode;
    int i = 0;
    for (; i < INODE_INNER_BLOCKS; i++){
        node->blocks[i] = INODE_EMPTY_BLOCK;
    }

    node->direct_pointers = INODE_EMPTY_BLOCK;
    node->indirect_pointers = INODE_EMPTY_BLOCK;
    node->size = 0;
    node->blocks_count = 0;

    return node;
}

void inode_unlink(bitmap_instance *bitmap, inode_t *node)
{
    bitmap_set(bitmap, node->id, 0);
}


void inode_free(inode_t *inode){
    free(inode);
}
