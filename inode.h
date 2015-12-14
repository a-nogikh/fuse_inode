#ifndef INODE_H_INCLUDED
#define INODE_H_INCLUDED

#define INODE_INNER_BLOCKS 10
#define INODE_EMPTY_BLOCK -1

typedef enum {INODE_FILE, INODE_DIRECTORY} inode_type;

struct inode{
    int id;
    inode_type type;
    int size, blocks_count;

    int blocks[INODE_INNER_BLOCKS];
    int level_1_block, level_2_block;
};
typedef struct inode inode;


#endif // INODE_H_INCLUDED
